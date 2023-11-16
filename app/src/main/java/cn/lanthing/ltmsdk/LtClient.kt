package cn.lanthing.ltmsdk

import android.util.Log
import android.view.Surface
import cn.lanthing.codec.LtMessage
import cn.lanthing.ltproto.ErrorCodeOuterClass
import cn.lanthing.ltproto.LtProto
import cn.lanthing.ltproto.signaling.JoinRoomAckProto.JoinRoomAck
import cn.lanthing.ltproto.signaling.JoinRoomProto.JoinRoom
import cn.lanthing.ltproto.signaling.SignalingMessageAckProto.SignalingMessageAck
import cn.lanthing.ltproto.signaling.SignalingMessageProto.SignalingMessage
import cn.lanthing.ltproto.signaling.SignalingMessageProto.SignalingMessage.RtcMessage
import cn.lanthing.net.SocketClient
import com.google.protobuf.ByteString
import com.google.protobuf.Message

// 1. 尽量让LtClient内部闭环，即尽量少点向App层回调东西
// 2. 向上回调用onMessage，不用回调函数，方便同步lanthing-pc的代码
// 3. 将来ltmsdk是要独立成一个aar/framework的，用同一份代码支持Android和iOS。它的全称是Lanthing Mobile SDK，不是Lanthing Android SDK😄

// lanthing-pc里所有东西都是C++写的，有rtc、信令、video、audio、input模块，然后这些模块都被一个client对象管理着
// 到了移动端，信令和input是Kotlin/Java写的，video、audio、rtc是C++写的
// 至于用于管理的client，一种直截了当的想法是用Kotlin/Java实现，video、audio、rtc暴露纯C接口，这么做是最接近lanthing-pc的写法
// 但是这样做有一个坏处，Kotlin/Java层与C++层的交互太多，比如rtc(c++)收到一帧视频，要回调到Kotlin/Java层，什么都不处理，马上又传递到video(c++)
// 这显然是不能接受的
// 所以只能把client分成两部分实现，一部分在Kotlin/Java实现，一部分在C++实现
class LtClient(
    private val videoSurface: Surface,
    private val cursorSurface: Surface,
    private val videoWidth: Int,
    private val videoHeight: Int,
    private val clientID: String,
    private val roomID: String,
    private val token: String,
    private val p2pUsername: String,
    private val p2pPassword: String,
    signalingAddress: String,
    signalingPort: Int,
    private val codecType: String,
    private val audioChannels: Int,
    private val audioFreq: Int,
    private val reflexServers: List<String>,
    private val onMessage: (msgType: UInt, message: Message) -> Unit //对应lanthing-pc ClientManager::onPipeMessage
) {

    private val signalingClient = SocketClient(
        host = signalingAddress,
        port = signalingPort,
        onConnected = this::onSignalingConnected,
        onDisconnected = this::onSignalingDisconnected,
        onMessage = this::onSignalingNetMessage)

    private var nativeClient: Long = 0

    init {
        nativeClient = createNativeClient( videoSurface, cursorSurface, videoWidth, videoHeight,
            clientID, roomID, token, p2pUsername, p2pPassword, signalingAddress, signalingPort,
            codecType, audioChannels, audioFreq, reflexServers
        )
    }

    fun ok(): Boolean {
        return nativeClient != 0L
    }


    // 对应lanthing-pc ClientSession::start()
    fun connect() {
        signalingClient.connect()
    }

    // 对应lanthing-pc Client::onPlatformExit()
    fun stop() {
        if (nativeClient != 0L) {
            nativeStop(nativeClient)
            destroyNativeClient(nativeClient)
            nativeClient = 0L
        }
    }

    private fun onSignalingConnected() {
        val msg = JoinRoom.newBuilder()
            .setRoomId(roomID)
            .setSessionId(clientID)
            .build();
        signalingClient.sendMessage(LtProto.JoinRoom.ID, msg)
    }

    private fun onSignalingDisconnected() {
        Log.e("ltmsdk", "Signaling disconnected")
        stop()
    }

    private fun onSignalingNetMessage(msg: LtMessage) {
        if (msg.protoMsg == null) {
            Log.e("ltmsdk", "Received LtMessage with null protoMsg, type = ${msg.type}")
            return
        }
        when (msg.type) {
            LtProto.JoinRoomAck.ID -> onJoinRoomAck(msg.protoMsg as JoinRoomAck)
            LtProto.SignalingMessage.ID -> onSignalingMessage(msg.protoMsg as SignalingMessage)
            LtProto.SignalingMessageAck.ID -> onSignalingMessageAck(msg.protoMsg as SignalingMessageAck)
            else -> Log.w("ltmsdk", "Unknown message type ${msg.type}")
        }
    }

    private fun onJoinRoomAck(msg: JoinRoomAck) {
        if (msg.errCode != ErrorCodeOuterClass.ErrorCode.Success) {
            Log.e("ltmsdk", "Join room $roomID with id $clientID failed")
            return
        }
        Log.i("ltmsdk", "Join room success")
        val success = nativeStart(nativeClient)
        if (!success) {
            //TODO: error handling
            Log.e("ltmsdk", "nativeStart failed")
        }
    }

    private fun onSignalingMessage(msg: SignalingMessage) {
        when (msg.level) {
            SignalingMessage.Level.Core -> dispatchSignalingMessageCore(msg.coreMessage)
            SignalingMessage.Level.Rtc -> dispatchSignalingMessageRtc(msg.rtcMessage)
            else -> {
                Log.e("ltmsdk", "Unknown SignalingMessage level: ${msg.level}")
            }
        }
    }

    private fun dispatchSignalingMessageCore(msg: SignalingMessage.CoreMessage) {
        if (msg.key.equals("close")) {
            stop()
        }
    }

    private fun dispatchSignalingMessageRtc(msg: SignalingMessage.RtcMessage) {
        nativeOnSignalingMessage(nativeClient, msg.key, msg.value.toByteArray())
    }

    private fun onSignalingMessageAck(msg: SignalingMessageAck) {
        when (msg.errCode) {
            ErrorCodeOuterClass.ErrorCode.Success -> {}
            ErrorCodeOuterClass.ErrorCode.SignalingPeerNotOnline -> {
                Log.e("ltmsdk", "Send signaling message failed, remote device not online")
            }
            else -> {
                Log.e("ltmsdk", "Send signaling message failed")
            }
        }
    }

    private fun onNativeClosed() {
        Log.i("ltmsdk", "LtClient onNativeClosed()")
    }

    private fun onNativeConnected() {
        Log.i("ltmsdk", "LtClient onNativeConnected()")
    }

    private fun onNativeSignalingMessage(key: String, value: ByteArray) {
        Log.i("ltmsdk", "LtClient onNativeSignalingMessage(key:'$key')")
        val byteValue = ByteString.copyFrom(value)
        val msg = SignalingMessage.newBuilder()
            .setLevel(SignalingMessage.Level.Rtc)
            .setRtcMessage(RtcMessage.newBuilder().setKey(key).setValue(byteValue).build())
            .build()
        signalingClient.sendMessage(LtProto.SignalingMessage.ID, msg)
    }

    private fun dummyFunc() {
        Log.i("ltmsdk", "LtClient.dummyFunc is called")
    }


    private external fun createNativeClient(videoSurface: Surface, cursorSurface: Surface, videoWidth: Int, videoHeight: Int,
                                            clientID: String, roomID: String, token: String,
                                            p2pUsername: String, p2pPassword: String, signalingAddress: String,
                                            signalingPort: Int, codecType: String, audioChannels: Int,
                                            audioFreq: Int, reflexServers: List<String>): Long
    private external fun destroyNativeClient(cli: Long)
    private external fun nativeStart(cli: Long): Boolean
    private external fun nativeStop(cli: Long)
    private external fun nativeSwitchMouseMode(cli: Long)
    private external fun nativeOnSignalingMessage(cli: Long, key: String, value: ByteArray)
}
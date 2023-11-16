package cn.lanthing.activity.main

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.runtime.toMutableStateList
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.integerResource
import cn.lanthing.R
import cn.lanthing.activity.stream.StreamActivity
import cn.lanthing.codec.LtMessage
import cn.lanthing.ltproto.ErrorCodeOuterClass
import cn.lanthing.ltproto.LtProto
import cn.lanthing.ltproto.common.StreamingParamsProto.StreamingParams
import cn.lanthing.ltproto.common.VideoCodecTypeProto.VideoCodecType
import cn.lanthing.ltproto.server.AllocateDeviceIDAckProto.AllocateDeviceIDAck
import cn.lanthing.ltproto.server.AllocateDeviceIDProto.AllocateDeviceID
import cn.lanthing.ltproto.server.ConnectionTypeProto
import cn.lanthing.ltproto.server.LoginDeviceAckProto.LoginDeviceAck
import cn.lanthing.ltproto.server.LoginDeviceProto
import cn.lanthing.ltproto.server.NewVersionProto.NewVersion
import cn.lanthing.ltproto.server.RequestConnectionAckProto.RequestConnectionAck
import cn.lanthing.ltproto.server.RequestConnectionProto.RequestConnection
import cn.lanthing.net.SocketClient

class MainActivity : ComponentActivity() {
    private val kHost: String = "192.168.31.121"
    private val kPort: Int = 9876
    private var deviceID: Long = 0 // 只做主控，这个ID是不显示给用户的
    private var deviceCookie: String = ""
    private var settings: SharedPreferences? = null
    private var requestFlying: Boolean = false
    private var versionMajor: Int = 0
    private var versionMinor: Int = 0
    private var versionPatch: Int = 0
    private val socketClient: SocketClient = SocketClient(
        kHost,
        kPort,
        onConnected = this::onConnected,
        onDisconnected = this::onDisconnected,
        onMessage = this::onMessage
    )
    private val kSettingsFilename: String = "lanthing_kv_settings"

    companion object {
        init {
            System.loadLibrary("rtc")
            System.loadLibrary("lanthing")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        settings = getSharedPreferences(kSettingsFilename, Context.MODE_PRIVATE)
        if (settings == null) {
            Log.e("main", "Get getSharedPreferences($kSettingsFilename) failed")
            // 会失败吗???
            return
        }
        deviceID = settings?.getLong("device_id", 0L) ?: 0
        socketClient.connect()
        setContent {
            // 没法移动到非Compose函数去
            versionMajor = integerResource(id = R.integer.version_major)
            versionMinor = integerResource(id = R.integer.version_minor)
            versionPatch = integerResource(id = R.integer.version_patch)
            Logging()
        }
    }

    // 跑在UI线程
    private fun requestConnection(peerID: Long, accessCode: String) {
        requestFlying = true
        val cookie = settings?.getString("to_$peerID", "")
        val msg = RequestConnection.newBuilder()
            .setRequestId(0)
            .setDeviceId(peerID)
            .setAccessToken(accessCode)
            .setConnType(ConnectionTypeProto.ConnectionType.Control)
        if (cookie != null) {
            msg.cookie = cookie
        }
        val params = StreamingParams.newBuilder()
            .setEnableDriverInput(false)
            .setEnableGamepad(false)
            .setScreenRefreshRate(60)
            .setVideoWidth(1920)
            .setVideoHeight(1080)
            .addVideoCodecs(VideoCodecType.HEVC)
            .addVideoCodecs(VideoCodecType.AVC)
        msg.streamingParams = params.build()
        socketClient.sendMessage(LtProto.RequestConnection.ID, msg.build())
        Handler(Looper.getMainLooper()).postDelayed({
            if (requestFlying) {
                requestFlying = false
                setContent {
                    ErrorMessage(errCode = ErrorCodeOuterClass.ErrorCode.RequestConnectionTimeout.number) {
                        setContent {
                            MainPage(this::requestConnection)
                        }
                    }
                }
            }
        }, 10_000)
        setContent {
            Connecting()
        }
    }

    private fun onConnected() {
        if (deviceID != 0L) {
            loginDevice()
        } else {
            allocateDeviceID()
        }
    }

    private fun onDisconnected() {
        runOnUiThread() {
            setContent {
                Logging()
            }
        }
    }

    private fun onMessage(msg: LtMessage) {
        if (msg.protoMsg == null) {
            Log.e("socket", "Received LtMessage with null protoMsg, type = ${msg.type}")
            return
        }
        runOnUiThread() {
            when (msg.type) {
                LtProto.LoginDeviceAck.ID -> onLoginDeviceAck(msg.protoMsg as LoginDeviceAck)
                LtProto.AllocateDeviceIDAck.ID -> onAllocateDeviceIDAck(msg.protoMsg as AllocateDeviceIDAck)
                LtProto.RequestConnectionAck.ID -> onRequestConnectionAck(msg.protoMsg as RequestConnectionAck)
                LtProto.NewVersion.ID -> onNewVersion(msg.protoMsg as NewVersion)
                else -> Log.w("main", "Unknown message type ${msg.type}")
            }
        }
    }

    private fun loginDevice() {
        val msg =
            LoginDeviceProto.LoginDevice.newBuilder()
                .setDeviceId(deviceID)
                .setCookie(deviceCookie)
                .setVersionMajor(versionMajor)
                .setVersionMinor(versionMinor)
                .setVersionPatch(versionPatch)
                .build()
        msg ?: return
        socketClient.sendMessage(LtProto.LoginDevice.ID, msg)
    }

    private fun onLoginDeviceAck(msg: LoginDeviceAck) {
        when (msg.errCode) {
            ErrorCodeOuterClass.ErrorCode.Success -> {
                Log.i("main", "LoginDevice success")
            }
            ErrorCodeOuterClass.ErrorCode.LoginDeviceInvalidStatus -> {
                setContent {
                    ErrorMessage(errCode = ErrorCodeOuterClass.ErrorCode.LoginDeviceInvalidStatus.number) {
                        setContent {
                            MainPage(this::requestConnection)
                        }
                    }
                }
                return
            }
            ErrorCodeOuterClass.ErrorCode.LoginDeviceInvalidID -> {
                Log.e("main", "LoginDevice failed, LoginDeviceInvalidID")
                if (msg.newDeviceId != 0L) {
                    deviceID = msg.newDeviceId
                    settings?.edit()?.putLong("device_id", msg.newDeviceId)?.apply()

                } else {
                    setContent {
                        ErrorMessage(errCode = ErrorCodeOuterClass.ErrorCode.LoginDeviceInvalidID.number) {
                            setContent {
                                MainPage(this::requestConnection)
                            }
                        }
                    }
                    return
                }
            }
            ErrorCodeOuterClass.ErrorCode.LoginDeviceInvalidCookie -> {
                Log.e("main", "LoginDevice failed, LoginDeviceInvalidCookie")
                if (msg.newDeviceId != 0L) {
                    deviceID = msg.newDeviceId
                    settings?.edit()?.putLong("device_id", msg.newDeviceId)?.apply()
                } else {
                    setContent {
                        ErrorMessage(errCode = ErrorCodeOuterClass.ErrorCode.LoginDeviceInvalidCookie.number) {
                            setContent {
                                MainPage(this::requestConnection)
                            }
                        }
                    }
                    return
                }
            }
            else -> {
                Log.e("main", "Unknown LoginDeviceAck error code ${msg.errCode}")
                return
            }
        }
        val editor = settings?.edit()
        if (!msg.newCookie.isNullOrEmpty() && editor != null) {
            editor.putString("device_cookie", msg.newCookie)
            editor.apply()
        }
        setContent {
            MainPage(this::requestConnection)
        }
    }

    private fun allocateDeviceID() {
        val msg = AllocateDeviceID.newBuilder().build()
        msg ?: return
        socketClient.sendMessage(LtProto.AllocateDeviceID.ID, msg)
    }

    private fun onAllocateDeviceIDAck(msg: AllocateDeviceIDAck) {
        val editor = settings?.edit() ?: return
        deviceCookie = settings?.getString("device_cookie", "") ?: ""
        if (deviceCookie.isEmpty()) {
            Log.e("main", "Received AllocateDeviceIDAck with empty device_cookie")
        } else {
            editor.putString("device_cookie", deviceCookie)
        }
        deviceID = msg.deviceId
        editor.putLong("device_id", deviceID)
        editor.apply()
        loginDevice()
    }

    private fun onRequestConnectionAck(msg: RequestConnectionAck) {
        if (!requestFlying) {
            Log.e("main", "Received RequestConnectionAck but too late")
            return
        }
        requestFlying = false
        if (msg.streamingParams.videoCodecsCount == 0) {
            Log.e("main", "Received RequestConnectionAck without video codecs")
            return
        }
        val codec = msg.streamingParams.videoCodecsList[0]
        val reflxs: ArrayList<String> = ArrayList()
        for (address in msg.reflexServersList) {
            reflxs.add(address)
        }
        setContent {
            val activity = LocalContext.current as Activity
            val bundle = Bundle()
            bundle.putString("clientID", msg.clientId)
            bundle.putString("roomID", msg.roomId)
            bundle.putString("token", msg.authToken)
            bundle.putString("p2pUsername", msg.p2PUsername)
            bundle.putString("p2pPassword", msg.p2PPassword)
            bundle.putString("signalingAddress", msg.signalingAddr)
            bundle.putInt("signalingPort", msg.signalingPort)
            bundle.putString("codecType", codec.toString().lowercase())
            bundle.putInt("audioChannels", msg.streamingParams.audioChannels)
            bundle.putInt("audioFreq", msg.streamingParams.audioSampleRate)
            bundle.putStringArrayList("reflexServers", reflxs)
            val intent = Intent(this@MainActivity, StreamActivity::class.java)
            intent.putExtras(bundle)
            activity.startActivity(intent)
        }

    }

    private fun onNewVersion(msg: NewVersion) {
        //TODO: show new version
    }
}

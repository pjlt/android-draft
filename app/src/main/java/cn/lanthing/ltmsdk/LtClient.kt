package cn.lanthing.ltmsdk

import android.view.Surface
import com.google.protobuf.Message

// 1. 尽量让LtClient内部闭环，即尽量少点向App层回调东西
// 2. 向上回调用onMessage，不用回调函数，方便同步lanthing-pc的代码
// 3. 将来ltmsdk是要独立曾一个aar/framework的，用同一份代码支持Android和iOS。它的全称是Lanthing Mobile SDK，不是Lanthing Android SDK😄
class LtClient(
    private val onMessage: (msgType: UInt, message: Message) -> Unit //对应lanthing-pc ClientManager::onPipeMessage
) {

    // 对应lanthing-pc ClientSession::start()
    fun connect(
        videoSurface: Surface,
        cursorSurface: Surface,
        clientID: String,
        roomID: String,
        token: String,
        p2pUsername: String,
        p2pPassword: String,
        signalingAddress: String,
        signalingPort: UShort,
        codecType: String,
        audioChannels: Int,
        audioFreq: Int,
        reflexServers: List<String>
    ) {
        // 1. 连接信令
        // 2. 创建rtc
        // lanthing-pc里所有东西都是C++写的，有rtc、信令、video、audio、input模块，然后这些模块都被一个client对象管理着
        // 到了移动端，信令和input是Kotlin/Java写的，video、audio、rtc是C++写的
        // 至于用于管理的client，一种直截了当的想法是用Kotlin/Java实现，video、audio、rtc暴露纯C接口，这么做是最接近lanthing-pc的写法
        // 但是这样做有一个坏处，Kotlin/Java层与C++层的交互太多，比如rtc(c++)收到一帧视频，要回调到Kotlin/Java层，什么都不处理，马上又传递到video(c++)
        // 这显然是不能接受的
        // 所以只能把client分成两部分实现，一部分在Kotlin/Java实现，一部分在C++实现
    }

    // 对应lanthing-pc Client::onPlatformExit()
    fun stop() {
        //
    }
}
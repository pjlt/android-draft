package cn.lanthing.net

import android.util.Log
import cn.lanthing.codec.LtCodec
import cn.lanthing.codec.LtMessage
import cn.lanthing.codec.Protocol
import cn.lanthing.ltproto.LtProto
import cn.lanthing.ltproto.common.KeepAliveProto.KeepAlive
import com.google.protobuf.Message
import io.netty.bootstrap.Bootstrap
import io.netty.channel.Channel
import io.netty.channel.ChannelHandlerContext
import io.netty.channel.ChannelInboundHandlerAdapter
import io.netty.channel.ChannelInitializer
import io.netty.channel.nio.NioEventLoopGroup
import io.netty.channel.socket.nio.NioSocketChannel
import io.netty.handler.ssl.SslContextBuilder
import io.netty.handler.ssl.SslHandler
import java.io.ByteArrayInputStream
import java.io.InputStream
import java.net.InetSocketAddress
import java.security.cert.CertificateFactory
import java.security.cert.X509Certificate
import java.util.concurrent.TimeUnit


class SocketClient(
    private val host: String,
    private val port: Int,
    private val certificate: String,
    private val onConnected: () -> Unit,
    private val onDisconnected: () -> Unit,
    private val onMessage: (msg: LtMessage) -> Unit
) : ChannelInboundHandlerAdapter() {

    private var channel: Channel? = null

    init {
        val msgTypes = ArrayList<LtCodec.MsgType>()
        for (msgType in LtProto.values()) {
            msgTypes.add(LtCodec.MsgType(msgType.ID, msgType.className))
        }
        LtCodec.initialize(msgTypes)
    }

    @Throws(Exception::class)
    override fun channelActive(ctx: ChannelHandlerContext) {
        channel = ctx.channel()
        ctx.channel().eventLoop()?.scheduleAtFixedRate(this::sendKeepAlive, 0, 10, TimeUnit.SECONDS)
        onConnected()
    }

    @Throws(Exception::class)
    override fun channelInactive(ctx: ChannelHandlerContext) {
        Log.e("net", "Reconnecting to $host:$port")
        onDisconnected()
        connect()
    }

    @Throws(Exception::class)
    override fun channelRead(ctx: ChannelHandlerContext, msg: Any) {
        val ltMessage = msg as LtMessage
        if (ltMessage.type == LtProto.KeepAliveAck.ID) {
            return
        }
        onMessage(ltMessage)
    }

    @Throws(Exception::class)
    override fun exceptionCaught(ctx: ChannelHandlerContext, cause: Throwable) {
        Log.e("net", "SocketClient caught exception $cause")
        onDisconnected()
        connect()
    }

    fun connect() {
        val loopGroup = NioEventLoopGroup(1)
        val bs = Bootstrap()
        bs.group(loopGroup)
            .channel(NioSocketChannel::class.java)
            .handler(object : ChannelInitializer<Channel>() {
                @Throws(Exception::class)
                override fun initChannel(ch: Channel) {
                    val certStream: InputStream = ByteArrayInputStream(certificate.toByteArray())
                    val x509 = CertificateFactory.getInstance("X509")
                        .generateCertificate(certStream) as X509Certificate
                    val sslContext = SslContextBuilder.forClient().trustManager(x509).build()
                    val sslEngine = sslContext.newEngine(ch.alloc())
                    sslEngine.useClientMode = true
                    val sslHandler = SslHandler(sslEngine)
                    //ch.pipeline().addFirst("ssl", sslHandler)
                    ch.pipeline().addLast("protocol", Protocol())
                    ch.pipeline().addLast("message", LtCodec())
                    ch.pipeline().addLast("client", this@SocketClient)
                }
            })
        loopGroup.execute { bs.connect(InetSocketAddress(host, port)) }

        Log.i("socket", String.format("Connecting to %s:%d", host, port))
    }

    fun sendMessage(msgID: Long, msg: Message) {
        val eventLoop = channel?.eventLoop() ?: return
        if (!eventLoop.inEventLoop()) {
            eventLoop.submit() {
                sendMessage(msgID, msg)
            }
            return
        }
        channel?.writeAndFlush(LtMessage(msgID, msg))
    }

    private fun sendKeepAlive() {
        val msg = KeepAlive.newBuilder().build();
        msg ?: return
        sendMessage(LtProto.KeepAlive.ID, msg)
    }
}
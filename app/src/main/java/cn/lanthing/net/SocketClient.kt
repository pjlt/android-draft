package cn.lanthing.net

import android.util.Log
import cn.lanthing.codec.LtCodec
import cn.lanthing.codec.LtMessage
import cn.lanthing.codec.Protocol
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


class SocketClient(
    private val host: String,
    private val port: Int,
    private val certificate: String,
    deviceID: Long,
    onConnected: Runnable,
    onDisconnected: Runnable
) : ChannelInboundHandlerAdapter() {

    @Throws(Exception::class)
    override fun channelActive(ctx: ChannelHandlerContext) {
        loginDevice()
    }

    @Throws(Exception::class)
    override fun channelInactive(ctx: ChannelHandlerContext) {
        //Reconnect
        Log.e("net", "Reconnecting to $host:$port")
        connect()
    }

    @Throws(Exception::class)
    override fun channelRead(ctx: ChannelHandlerContext, msg: Any) {
        dispatchMessage(msg as LtMessage)
    }

    @Throws(Exception::class)
    override fun exceptionCaught(ctx: ChannelHandlerContext, cause: Throwable) {
        //Reconnect
        Log.e("net", "SocketClient caught exception $cause")
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
                    ch.pipeline().addFirst("ssl", sslHandler)
                    ch.pipeline().addLast("protocol", Protocol())
                    ch.pipeline().addLast("message", LtCodec())
                    ch.pipeline().addLast("client", this@SocketClient)
                }
            })
        bs.connect(InetSocketAddress(host, port))
        Log.i("socket", String.format("Connecting to %s:%d", host, port))
    }

    private fun dispatchMessage(msg: LtMessage) {
        when (msg.type) {

        }
    }

    private fun loginDevice() {
        //
    }

    private fun onLoginDeviceAck() {
        //
    }

}
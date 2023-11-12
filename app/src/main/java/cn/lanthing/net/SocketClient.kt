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
    private val onConnected: () -> Unit,
    private val onDisconnected: () -> Unit,
    private val onMessage: (msg: LtMessage) -> Unit
) : ChannelInboundHandlerAdapter() {
    private val kCertificate: String = """-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----"""

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
                    val certStream: InputStream = ByteArrayInputStream(kCertificate.toByteArray())
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
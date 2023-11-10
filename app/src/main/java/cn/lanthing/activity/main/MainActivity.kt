@file:OptIn(ExperimentalMaterial3Api::class)

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
import androidx.compose.material3.ExperimentalMaterial3Api
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
-----END CERTIFICATE-----""";
    private val kHost: String = "lanthing.net";
    private val kPort: Int = 44898;
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
        kCertificate,
        onConnected = this::onConnected,
        onDisconnected = this::onDisconnected,
        onMessage = this::onMessage
    )
    private val kSettingsFilename: String = "lanthing_kv_settings"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        settings = getSharedPreferences(kSettingsFilename, Context.MODE_PRIVATE)
        if (settings == null) {
            Log.e("main", "Get getSharedPreferences($kSettingsFilename) failed")
            // 会失败吗???
            return
        }
        deviceID = settings?.getLong("device_id", 0L) ?: 0
        socketClient.connect();
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
            msg.setCookie(cookie)
        }
        val params = StreamingParams.newBuilder()
            .setEnableDriverInput(false)
            .setEnableGamepad(false)
            .setScreenRefreshRate(60)
            .setVideoWidth(1920)
            .setVideoHeight(1080)
            .addVideoCodecs(VideoCodecType.HEVC)
            .addVideoCodecs(VideoCodecType.AVC)
        msg.setStreamingParams(params.build())
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
            allocateDeviceID();
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
                .build();
        msg ?: return
        socketClient.sendMessage(LtProto.LoginDevice.ID, msg)
    }

    private fun onLoginDeviceAck(msg: LoginDeviceAck) {
        when (msg.errCode) {
            ErrorCodeOuterClass.ErrorCode.Success -> {
                Log.e("main", "LoginDevice success")
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
        val msg = AllocateDeviceID.newBuilder().build();
        msg ?: return
        socketClient.sendMessage(LtProto.AllocateDeviceID.ID, msg)
    }

    private fun onAllocateDeviceIDAck(msg: AllocateDeviceIDAck) {
        val editor = settings?.edit() ?: return
        deviceCookie = settings?.getString("device_cookie", "") ?: "";
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
        setContent {
            val activity = LocalContext.current as Activity
            activity.startActivity(Intent(this@MainActivity, StreamActivity::class.java))
        }

    }

    private fun onNewVersion(msg: NewVersion) {
        //TODO: show new version
    }
}

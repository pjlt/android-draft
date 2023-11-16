package cn.lanthing.activity.stream

import android.content.pm.ActivityInfo
import android.os.Bundle
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView
import cn.lanthing.ltmsdk.LtClient
import com.google.protobuf.Message

class StreamActivity : ComponentActivity() {
    private var videoWidth: Int = 0
    private var videoHeight: Int = 0
    private var videoHolder: SurfaceHolder? = null
    private var cursorHolder: SurfaceHolder? = null
    private var ltClientStarted: Boolean = false
    private lateinit var clientID: String
    private lateinit var roomID: String
    private lateinit var token: String
    private lateinit var p2pUsername: String
    private lateinit var p2pPassword: String
    private lateinit var signalingAddress: String
    private var signalingPort: Int = 0
    private lateinit var codecType: String
    private var audioChannels: Int = 0
    private var audioFreq: Int = 0
    private var reflexServers: ArrayList<String>? = null
    private lateinit var ltClient: LtClient
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        val params = intent.extras
        if (params == null || params.isEmpty) {
            finish()
            return
        } else {
            videoWidth = params.getInt("videoWidth", 0)
            videoHeight = params.getInt("videoHeight", 0)
            clientID = params.getString("clientID", "")
            roomID = params.getString("roomID", "")
            token = params.getString("token", "")
            p2pUsername = params.getString("p2pUsername", "")
            p2pPassword = params.getString("p2pPassword", "")
            signalingAddress = params.getString("signalingAddress", "")
            signalingPort = params.getInt("signalingPort", 0)
            codecType = params.getString("codecType", "")
            audioChannels = params.getInt("audioChannels", 0)
            audioFreq = params.getInt("audioFreq", 0)
            reflexServers = params.getStringArrayList("reflexServers")
            if (clientID.isEmpty() || roomID.isEmpty() || token.isEmpty() || p2pUsername.isEmpty()
                || p2pPassword.isEmpty() || signalingAddress.isEmpty() || signalingPort == 0
                || (codecType != "avc" && codecType != "hevc") || audioChannels == 0
                || audioFreq == 0 || reflexServers.isNullOrEmpty()) {
                Log.e("stream", "Invalid stream parameters: $params")
                finish()
            }
        }
        setContent {
            AndroidView(modifier = Modifier.fillMaxSize(), factory = { ctx ->
                SurfaceView(ctx).apply {
                    holder.addCallback(object : SurfaceHolder.Callback {
                        override fun surfaceCreated(holder: SurfaceHolder) {
                            Log.i("stream", "Video surface created")
                        }

                        override fun surfaceChanged(
                            holder: SurfaceHolder,
                            format: Int,
                            width: Int,
                            height: Int
                        ) {
                            Log.i("stream", "Video surface changed format:$format, width:$width, height:$height")
                            this@StreamActivity.videoHolder = holder
                            if (this@StreamActivity.cursorHolder != null && !ltClientStarted) {
                                ltClientStarted = true
                                createLtClient()
                            }
                        }

                        override fun surfaceDestroyed(holder: SurfaceHolder) {
                            Log.i("stream", "Video surface destroyed")
                            //TODO: 停止串流
                            this@StreamActivity.videoHolder = null
                        }
                    })
                }
            })
            AndroidView(factory = { ctx -> SurfaceView(ctx).apply {
                holder.addCallback(object : SurfaceHolder.Callback {
                    override fun surfaceCreated(holder: SurfaceHolder) {
                        Log.i("stream", "Cursor surface created")
                    }

                    override fun surfaceChanged(
                        holder: SurfaceHolder,
                        format: Int,
                        width: Int,
                        height: Int
                    ) {
                        Log.i("stream", "Cursor surface changed format:$format, width:$width, height:$height")
                        this@StreamActivity.cursorHolder = holder
                        if (this@StreamActivity.videoHolder != null && !ltClientStarted) {
                            ltClientStarted = true
                            createLtClient()
                        }
                    }

                    override fun surfaceDestroyed(holder: SurfaceHolder) {
                        Log.i("stream", "Cursor surface destroyed")
                        this@StreamActivity.cursorHolder = null
                    }
                })
            }})
        }
    }

    private fun createLtClient() {
        val rflxs = reflexServers
        if (rflxs.isNullOrEmpty()) {
            //TODO: XXX
            finish()
            return
        }
        val videoSurface = videoHolder?.surface
        val cursorSurface = cursorHolder?.surface
        if (videoSurface == null || cursorSurface == null) {
            finish()
            return
        }
        ltClient = LtClient(
            videoSurface = videoSurface,
            cursorSurface = cursorSurface,
            videoWidth = videoWidth,
            videoHeight = videoHeight,
            clientID = clientID,
            roomID = roomID,
            token = token,
            p2pUsername = p2pUsername,
            p2pPassword = p2pPassword,
            signalingAddress = signalingAddress,
            signalingPort = signalingPort,
            codecType = codecType,
            audioChannels = audioChannels,
            audioFreq = audioFreq,
            reflexServers = rflxs,
            onMessage = this::onLtClientMessage
        )
        if (!ltClient.ok()) {
            Log.e("stream", "Create LtClient failed")
            finish()
            return
        }
        ltClient.connect()
    }

    private fun onLtClientMessage(msgType: UInt, msg: Message) {
        //
    }
}


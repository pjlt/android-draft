package cn.lanthing.activity.stream

import android.content.Context
import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.viewinterop.AndroidView
import cn.lanthing.ui.theme.AppTheme

class StreamActivity : ComponentActivity() {

    private var width: Int = 0
    private var height: Int = 0
    private var videoHolder: SurfaceHolder? = null
    private var cursorHolder: SurfaceHolder? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            AndroidView(factory = { ctx ->
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
                            this@StreamActivity.width = width
                            this@StreamActivity.height = height
                            this@StreamActivity.videoHolder = holder
                        }

                        override fun surfaceDestroyed(holder: SurfaceHolder) {
                            Log.i("stream", "Video surface destroyed")
                            //TODO: 停止串流
                            this@StreamActivity.width = 0
                            this@StreamActivity.height = 0
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
                    }

                    override fun surfaceDestroyed(holder: SurfaceHolder) {
                        Log.i("stream", "Cursor surface destroyed")
                        this@StreamActivity.cursorHolder = null
                    }
                })
            }})
        }
    }
}


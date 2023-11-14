package cn.lanthing.ltmsdk

import android.graphics.SurfaceTexture
import android.view.Surface

//暂时不用这种方式
class RenderTexture(glTexture: Int) : SurfaceTexture.OnFrameAvailableListener {

    private var surface: Surface;
    private var surfaceTexture: SurfaceTexture;

    init {
        surfaceTexture = SurfaceTexture(glTexture)
        surfaceTexture.setDefaultBufferSize(1920, 1080)
        surface = Surface(surfaceTexture)
    }

    override fun onFrameAvailable(p0: SurfaceTexture?) {
        TODO("Not yet implemented")
    }

    fun updateRenderTexture() {
        surfaceTexture.updateTexImage()
    }
}
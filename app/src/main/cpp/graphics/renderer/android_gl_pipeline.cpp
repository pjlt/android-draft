/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2023 Zhennan Tu <zhennan.tu@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "android_gl_pipeline.h"

#include <array>

#include <ltlib/logging.h>

// 大体跟lanthing-pc的VaGlPipeline一致，以后要合并相同部分

namespace {
struct AutoGuard {
    AutoGuard(const std::function<void()>& func)
        : func_{func} {}
    ~AutoGuard() {
        if (func_) {
            func_();
        }
    }

private:
    std::function<void()> func_;
};
} // namespace

namespace lt {

AndroidGlPipeline::AndroidGlPipeline(const Params& params)
    : a_native_window_{reinterpret_cast<ANativeWindow*>(params.window)}
    , video_width_{params.width}
    , video_height_{params.height} {}

AndroidGlPipeline::~AndroidGlPipeline() {
    if (egl_display_) {
        eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl_context_) {
            eglDestroyContext(egl_display_, egl_context_);
        }
        if (egl_surface_) {
            eglDestroySurface(egl_display_, egl_surface_);
        }
        eglTerminate(egl_display_);
    }
}

bool AndroidGlPipeline::init() {
    window_width_ = ANativeWindow_getWidth(a_native_window_);
    window_height_ = ANativeWindow_getHeight(a_native_window_);
    if (!loadFuncs()) {
        return false;
    }
    if (!initEGL()) {
        return false;
    }
    if (!initOpenGLES()) {
        return false;
    }
    EGLBoolean egl_ret =
        eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (egl_ret != EGL_TRUE) {
        LOG(ERR) << "eglMakeCurrent(null) return " << egl_ret << " error: " << eglGetError();
        return false;
    }
    return true;
}

bool AndroidGlPipeline::bindTextures(const std::vector<void*>& textures) {
    (void)textures;
    return true;
}

VideoRenderer::RenderResult AndroidGlPipeline::render(int64_t frame) {
    EGLBoolean egl_ret = eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    if (egl_ret != EGL_TRUE) {
        LOG(ERR) << "eglMakeCurrent return " << egl_ret << " error: " << eglGetError();
        return RenderResult::Failed;
    }
    AutoGuard ag{[this]() {
        EGLBoolean egl_ret =
                eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl_ret != EGL_TRUE) {
            LOG(ERR) << "eglMakeCurrent(null) return " << egl_ret << " error: " << eglGetError();
        }
    }};
    // TODO: xxx
}

void AndroidGlPipeline::updateCursor(int32_t cursor_id, float x, float y, bool visible) {
    (void)cursor_id;
    (void)x;
    (void)y;
    (void)visible;
}

void AndroidGlPipeline::switchMouseMode(bool absolute) {
    (void)absolute;
}

void AndroidGlPipeline::resetRenderTarget() {}

bool AndroidGlPipeline::present() {
    return true;
}

bool AndroidGlPipeline::waitForPipeline(int64_t max_wait_ms) {
    (void)max_wait_ms;
    return true;
}

void* AndroidGlPipeline::hwDevice() {
    //TODO: 返回与解码有关的硬件handle
    return nullptr;
}

void* AndroidGlPipeline::hwContext() {
    //TODO: 返回与解码有关的硬件handle
    return nullptr;
}

uint32_t AndroidGlPipeline::displayWidth() {
    return window_width_;
}

uint32_t AndroidGlPipeline::displayHeight() {
    return window_height_;
}

bool AndroidGlPipeline::loadFuncs() {
    eglCreateImageKHR_ =
        reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    if (eglCreateImageKHR_ == nullptr) {
        LOG(ERR) << "eglGetProcAddress(eglCreateImageKHR) failed";
        return false;
    }
    eglDestroyImageKHR_ =
        reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (eglDestroyImageKHR_ == nullptr) {
        LOG(ERR) << "eglGetProcAddress(eglDestroyImageKHR) failed";
        return false;
    }
    glEGLImageTargetTexture2DOES_ = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
        eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (glEGLImageTargetTexture2DOES_ == nullptr) {
        LOG(ERR) << "eglGetProcAddress(glEGLImageTargetTexture2DOES) failed";
        return false;
    }
    glGenVertexArraysOES_ =
        reinterpret_cast<PFNGLGENVERTEXARRAYSOESPROC>(eglGetProcAddress("glGenVertexArraysOES"));
    if (glGenVertexArraysOES_ == nullptr) {
        LOG(ERR) << "eglGetProcAddress(glGenVertexArrays) failed";
        return false;
    }
    glBindVertexArrayOES_ =
        reinterpret_cast<PFNGLBINDVERTEXARRAYOESPROC>(eglGetProcAddress("glBindVertexArrayOES"));
    if (glBindVertexArrayOES_ == nullptr) {
        LOG(ERR) << "eglGetProcAddress(glBindVertexArray) failed";
        return false;
    }
    return true;
}

bool AndroidGlPipeline::initEGL() {
    egl_display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_display_ == EGL_NO_DISPLAY) {
        LOG(ERR) << "eglGetDisplay failed";
        return false;
    }
    if (!eglInitialize(egl_display_, nullptr, nullptr)) {
        LOG(ERR) << "eglInitialize failed";
        return false;
    }
    if (!eglBindAPI(EGL_OPENGL_API)) {
        LOG(ERR) << "eglBindAPI failed";
        return false;
    }
    EGLint visual_attr[] = {EGL_SURFACE_TYPE,
                            EGL_WINDOW_BIT,
                            EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            EGL_OPENGL_ES2_BIT,
                            EGL_NONE};
    EGLConfig egl_cfg{};
    EGLint egl_cfg_count{};
    EGLBoolean egl_ret = eglChooseConfig(egl_display_, visual_attr, &egl_cfg, 1, &egl_cfg_count);
    if (!egl_ret || egl_cfg_count < 1) {
        LOG(ERR) << "eglChooseConfig failed, egl_ret:" << egl_ret
                 << ", egl_cfg_count:" << egl_cfg_count;
        return false;
    }
    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_cfg, a_native_window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        LOG(ERR) << "eglCreateWindowSurface failed";
        return false;
    }
    constexpr EGLint CORE_PROFILE_MAJOR_VERSION = 3;
    constexpr EGLint CORE_PROFILE_MINOR_VERSION = 3;
    EGLint egl_ctx_attr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    egl_context_ = eglCreateContext(egl_display_, egl_cfg, EGL_NO_CONTEXT, egl_ctx_attr);
    if (egl_context_ == EGL_NO_CONTEXT) {
        LOG(ERR) << "eglCreateContext failed";
        return false;
    }
    egl_ret = eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    if (egl_ret != EGL_TRUE) {
        LOG(ERR) << "eglMakeCurrent failed: " << eglGetError();
        return false;
    }
    egl_ret = eglSwapInterval(egl_display_, 0);
    if (egl_ret != EGL_TRUE) {
        LOG(ERR) << "eglSwapInterval failed: " << eglGetError();
        return false;
    }
    return true;
}

bool AndroidGlPipeline::initOpenGLES() {
    LOGF(INFO, "OpenGL vendor:   %s\n", glGetString(GL_VENDOR));
    LOGF(INFO, "OpenGL renderer: %s\n", glGetString(GL_RENDERER));
    LOGF(INFO, "OpenGL version:  %s\n", glGetString(GL_VERSION));

    GLuint vao;
    glGenVertexArraysOES_(1, &vao);
    glBindVertexArrayOES_(vao);
    const char* kVertexShader = R"(
#version 130
const vec2 coords[4] = vec2[]( vec2(0.,0.), vec2(1.,0.), vec2(0.,1.), vec2(1.,1.) );
uniform vec2 uTexCoordScale;
out vec2 vTexCoord;
void main() {
    vec2 c = coords[gl_VertexID];
    vTexCoord = c * uTexCoordScale;
    gl_Position = vec4(c * vec2(4.,-4.) + vec2(-1.,1.), 0., 1.);
}
)";
    const char* kFragmentShader = R"(
#version 130
in vec2 vTexCoord;
uniform sampler2D uTexY, uTexC;
const mat4 yuv2rgb = mat4(
    vec4(  1.1643835616,  1.1643835616,  1.1643835616,  0.0 ),
    vec4(  0.0, -0.2132486143,  2.1124017857,  0.0 ),
    vec4(  1.7927410714, -0.5329093286,  0.0,  0.0 ),
    vec4( -0.9729450750,  0.3014826655, -1.1334022179,  1.0 ));
out vec4 oColor;
void main() {
    oColor = yuv2rgb * vec4(texture(uTexY, vTexCoord).x,
                            texture(uTexC, vTexCoord).xy, 1.);
}
)";
    shader_ = glCreateProgram();
    if (!shader_) {
        LOG(ERR) << "glCreateProgram failed: " << glGetError();
        return false;
    }
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    if (!vs) {
        LOG(ERR) << "glCreateShader(GL_VERTEX_SHADER) failed: " << glGetError();
        return false;
    }
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (!fs) {
        LOG(ERR) << "glCreateShader(GL_FRAGMENT_SHADER) failed: " << glGetError();
        glDeleteShader(vs);
        return false;
    }
    glShaderSource(vs, 1, &kVertexShader, nullptr);
    glShaderSource(fs, 1, &kFragmentShader, nullptr);
    while (glGetError()) {
    }
    std::array<char, 512> buffer{0};
    GLint status;
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderInfoLog(vs, buffer.size(), nullptr, buffer.data());
        LOG(ERR) << "glCompileShader(GL_VERTEX_SHADER) failed: " << buffer.data();
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderInfoLog(vs, buffer.size(), nullptr, buffer.data());
        LOG(ERR) << "glCompileShader(GL_FRAGMENT_SHADER) failed: " << buffer.data();
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }
    glAttachShader(shader_, vs);
    glAttachShader(shader_, fs);
    glLinkProgram(shader_);
    glGetProgramiv(shader_, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderInfoLog(vs, buffer.size(), nullptr, buffer.data());
        LOG(ERR) << "glLinkProgram() failed: " << buffer.data();
        glDeleteProgram(shader_);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }
    glUseProgram(shader_);
    glUniform1i(glGetUniformLocation(shader_, "uTexY"), 0);
    glUniform1i(glGetUniformLocation(shader_, "uTexC"), 1);
    glGenTextures(2, textures_);
    for (int i = 0; i < 2; ++i) {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, textures_[i]);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    return true;
}

} // namespace lt
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

#include "android_dummy_renderer.h"

#include <ltlib/logging.h>

namespace lt {

AndroidDummyRenderer::AndroidDummyRenderer(const Params& params)
    : a_native_window_{reinterpret_cast<ANativeWindow*>(params.window)}
    , video_width_{params.width}
    , video_height_{params.height} {}

AndroidDummyRenderer::~AndroidDummyRenderer() {}

bool AndroidDummyRenderer::init() {
    window_width_ = ANativeWindow_getWidth(a_native_window_);
    window_height_ = ANativeWindow_getHeight(a_native_window_);
}

bool AndroidDummyRenderer::bindTextures(const std::vector<void*>& textures) {
    (void)textures;
    return true;
}

VideoRenderer::RenderResult AndroidDummyRenderer::render(int64_t frame) {
    return RenderResult::Success2;
}

void AndroidDummyRenderer::updateCursor(int cursor_id, float x, float y, bool visible) {
    (void)cursor_id;
    (void)x;
    (void)y;
    (void)visible;
}

void AndroidDummyRenderer::switchMouseMode(bool absolute) {
    (void)absolute;
}

void AndroidDummyRenderer::resetRenderTarget() {}

bool AndroidDummyRenderer::present() {
    return true;
}

bool AndroidDummyRenderer::waitForPipeline(int64_t max_wait_ms) {
    (void)max_wait_ms;
    return true;
}

void* AndroidDummyRenderer::hwDevice() {
    return nullptr;
}

void* AndroidDummyRenderer::hwContext() {
    return nullptr;
}

uint32_t AndroidDummyRenderer::displayWidth() {
    return window_width_;
}

uint32_t AndroidDummyRenderer::displayHeight() {
    return window_height_;
}

} // namespace lt
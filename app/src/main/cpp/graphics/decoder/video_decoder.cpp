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

#include "video_decoder.h"

#include <ltlib/logging.h>

#include "ndk_video_decoder.h"

namespace lt {

std::unique_ptr<VideoDecoder> VideoDecoder::create(const Params& params) {
    if (params.va_type != VaType::AndroidDummy) {
        LOG(ERR) << "Only support VaType::AndroidDummy";
        return nullptr;
    }
    NdkVideoDecoder::Params ndk_params{};
    ndk_params.hw_device = params.hw_device;
    ndk_params.hw_context = params.hw_context;
    ndk_params.height = params.height;
    ndk_params.width = params.width;
    ndk_params.codec_type = params.codec_type;
    std::unique_ptr<NdkVideoDecoder> decoder {new NdkVideoDecoder(ndk_params)};
    if (!decoder->init()) {
        return nullptr;
    }
    return decoder;
}

VideoDecoder::VideoDecoder(const Params& params)
    : codec_type_{params.codec_type}
    , width_{params.width}
    , height_{params.height} {}

VideoCodecType VideoDecoder::codecType() const {
    return codec_type_;
}

uint32_t VideoDecoder::width() const {
    return width_;
}

uint32_t VideoDecoder::height() const {
    return height_;
}

} // namespace lt
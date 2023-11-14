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

#include "ndk_video_decoder.h"

#include <ltlib/logging.h>
#include <ltlib/times.h>

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

NdkVideoDecoder::NdkVideoDecoder(const VideoDecoder::Params& params)
    : VideoDecoder(params)
    , a_native_window_(reinterpret_cast<ANativeWindow*>(params.hw_context)) {}

NdkVideoDecoder::~NdkVideoDecoder() {}

bool NdkVideoDecoder::init() {
    AMediaFormat* media_format = AMediaFormat_new();
    if (media_format == nullptr) {
        LOG(ERR) << "AMediaFormat_new failed";
    }
    AutoGuard ag{[&media_format]() { AMediaFormat_delete(media_format); }};
    switch (codecType()) {
    case lt::VideoCodecType::H264:
        media_codec_ = AMediaCodec_createDecoderByType("video/avc");
        AMediaFormat_setString(media_format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        break;
    case lt::VideoCodecType::H265:
        media_codec_ = AMediaCodec_createDecoderByType("video/hevc");
        AMediaFormat_setString(media_format, AMEDIAFORMAT_KEY_MIME, "video/hevc");
        break;
    default:
        LOG(ERR) << "Unknown video codec type " << (int)codecType();
        return false;
    }
    if (media_codec_ == nullptr) {
        LOGF(ERR, "AMediaCodec_createDecoderByType(%d) failed", (int)codecType());
        return false;
    }

    AMediaFormat_setInt32(media_format, AMEDIAFORMAT_KEY_WIDTH, static_cast<int32_t>(width()));
    AMediaFormat_setInt32(media_format, AMEDIAFORMAT_KEY_HEIGHT, static_cast<int32_t>(height()));
    AMediaFormat_setInt32(media_format, AMEDIAFORMAT_KEY_FRAME_RATE, 60);
    media_status_t status =
        AMediaCodec_configure(media_codec_, media_format, a_native_window_, nullptr, 0);
    if (status != AMEDIA_OK) {
        LOG(ERR) << "AMediaCodec_configure failed " << status;
        return false;
    }
    status = AMediaCodec_start(media_codec_);
    if (status != AMEDIA_OK) {
        LOG(ERR) << "AMediaCodec_start failed " << status;
        return false;
    }
    return true;
}

DecodedFrame NdkVideoDecoder::decode(const uint8_t* data, uint32_t size) {
    // 找到的所有教程都不是这么写的，不知道有没有问题😅
    DecodeStatus status = pushFrame(data, size);
    if (status != DecodeStatus::Success2) {
        DecodedFrame frame{};
        frame.status = status;
        return frame;
    }
    return pullFrame();
}

std::vector<void*> NdkVideoDecoder::textures() {
    return {};
}

DecodeStatus NdkVideoDecoder::pushFrame(const uint8_t* data, uint32_t size) {
    ssize_t index = AMediaCodec_dequeueInputBuffer(media_codec_, -1);
    if (index < 0) {
        return DecodeStatus::Failed;
    }
    size_t buff_size;
    uint8_t* buff =
            AMediaCodec_getInputBuffer(media_codec_, static_cast<size_t>(index), &buff_size);
    if (buff == nullptr || buff_size == 0) {
        LOG(ERR) << "AMediaCodec_getInputBuffer failed buff:" << buff
                 << ", buff_size:" << buff_size;
        return DecodeStatus::Failed;
    }
    if (buff_size < size) {
        LOGF(ERR, "Decode failed AMediaCodec buffer size(%zu) < input frame size(%u)", buff_size,
             size);
        return DecodeStatus::Failed;
    }
    ::memcpy(buff, data, size);
    // TODO: 确认C++的时间戳和JVM的一致
    auto pts = static_cast<uint64_t>(ltlib::steady_now_us());
    AMediaCodec_queueInputBuffer(media_codec_, index, 0, size, pts, 0);
    return DecodeStatus::Success2;
}

DecodedFrame NdkVideoDecoder::pullFrame() {
    // emmmm......
    DecodedFrame frame{};
    AMediaCodecBufferInfo info{};
    ssize_t index = AMediaCodec_dequeueOutputBuffer(media_codec_, &info, 1'000'000);
    if (index < 0) {
        LOG(ERR) << "AMediaCodec_dequeueOutputBuffer failed, index:" << index;
        frame.status = DecodeStatus::Failed;
        return frame;
    }
    AMediaCodec_releaseOutputBuffer(media_codec_, index, true);
    frame.status = DecodeStatus::Success2;
    frame.frame = 1;
    return frame;
}

} // namespace lt
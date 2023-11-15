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

#include "native_client.h"

#include <ltlib/logging.h>

namespace {

lt::VideoCodecType to_ltrtc(std::string codec_str) {
    static const std::string kAVC = "avc";
    static const std::string kHEVC = "hevc";
    std::transform(codec_str.begin(), codec_str.end(), codec_str.begin(),
                   [](char c) -> char { return (char)std::tolower(c); });
    if (codec_str == kAVC) {
        return lt::VideoCodecType::H264;
    }
    else if (codec_str == kHEVC) {
        return lt::VideoCodecType::H265;
    }
    else {
        return lt::VideoCodecType::Unknown;
    }
}

} // namespace

namespace lt {

bool LtNativeClient::Params::validate() const {
    if (client_id.empty() || room_id.empty() || token.empty() || p2p_username.empty() ||
        p2p_password.empty() || signaling_address.empty()) {
        return false;
    }
    if (signaling_port <= 0 || signaling_port > 65535) {
        return false;
    }
    if (codec != "avc" && codec != "hevc") {
        return false;
    }
    if (audio_channels <= 0 || audio_freq <= 0) {
        return false;
    }
    return true;
}

LtNativeClient::LtNativeClient(const Params& params)
    : auth_token_{params.token}
    , p2p_username_{params.p2p_username}
    , p2p_password_{params.p2p_password}
    , video_params_{to_ltrtc(params.codec), params.width, params.height, params.screen_refresh_rate,
                    std::bind(&LtNativeClient::sendMessageToHost, this, std::placeholders::_1,
                              std::placeholders::_2, std::placeholders::_3)}
    , audio_params_{AudioCodecType::PCM, static_cast<uint32_t>(params.audio_freq),
                    static_cast<uint32_t>(params.audio_channels)}
    , reflex_servers_{params.reflex_servers} {}

LtNativeClient::~LtNativeClient() {
    // LtNativeClient和lanthing-pc的Client的线程模型是不一样的，析构要小心处理
}

bool LtNativeClient::start() {
    hb_thread_ = ltlib::TaskThread::create("heart_beat");
    should_exit_ = false;
    if (!initTransport()) {
        LOG(INFO) << "Initialize rtc failed";
        return false;
    }
    LOG(INFO) << "Initialize rtc success";
    return true;
}

bool LtNativeClient::initTransport() { return true; }

void LtNativeClient::dispatchRemoteMessage(
    uint32_t type, const std::shared_ptr<google::protobuf::MessageLite>& msg) {}

void LtNativeClient::sendKeepAlive() {}
void LtNativeClient::onKeepAliveAck() {}
bool LtNativeClient::sendMessageToHost(uint32_t type,
                                       const std::shared_ptr<google::protobuf::MessageLite>& msg,
                                       bool reliable) { return  true;}

} // namespace lt
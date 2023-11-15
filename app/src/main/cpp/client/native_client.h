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

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>

#include <google/protobuf/message_lite.h>

#include <ltlib/time_sync.h>
#include <ltlib/threads.h>

#include <audio/player/audio_player.h>
#include <graphics/drpipeline/video_decode_render_pipeline.h>
#include <client/jvm_client_proxy.h>

namespace lt {

class LtNativeClient {
public:
    struct Params {
        int64_t jvm_client;
        std::string client_id;
        std::string room_id;
        std::string token;
        std::string p2p_username;
        std::string p2p_password;
        std::string signaling_address;
        int32_t signaling_port;
        std::string codec;
        uint32_t width;
        uint32_t height;
        uint32_t screen_refresh_rate;
        int32_t audio_channels;
        int32_t audio_freq;
        std::vector<std::string> reflex_servers;

        bool validate() const;
    };

public:
    static LtNativeClient* create(const Params& params);
    static void destroy(LtNativeClient* obj);
    ~LtNativeClient();

    bool start();
    void switchMouseMode();

private:
    LtNativeClient(const Params& params);

    void postTask(const std::function<void()>& task);
    void postDelayTask(int64_t delay_ms, const std::function<void()>& task);
    void checkWorkerTimeout();
    void syncTime();
    void tellAppKeepAliveTimeout();

    // transport
    bool initTransport();
    static void onTpData(void* user_data, const uint8_t* data, uint32_t size, bool is_reliable);
    static void onTpVideoFrame(void* user_data, const lt::VideoFrame& frame);
    static void onTpAudioData(void* user_data, const lt::AudioData& audio_data);
    static void onTpConnected(void* user_data, lt::LinkType link_type);
    static void onTpConnChanged(void* user_data /*old_conn_info, new_conn_info*/);
    static void onTpFailed(void* user_data);
    static void onTpDisconnected(void* user_data);
    static void onTpSignalingMessage(void* user_data, const char* key, const char* value);
    // 数据通道.
    void dispatchRemoteMessage(uint32_t type,
                               const std::shared_ptr<google::protobuf::MessageLite>& msg);
    void sendKeepAlive();
    void onKeepAliveAck();
    bool sendMessageToHost(uint32_t type, const std::shared_ptr<google::protobuf::MessageLite>& msg,
                           bool reliable);
    void onStartTransmissionAck(const std::shared_ptr<google::protobuf::MessageLite>& msg);
    void onTimeSync(std::shared_ptr<google::protobuf::MessageLite> msg);
    void onSendSideStat(std::shared_ptr<google::protobuf::MessageLite> msg);
    void onCursorInfo(std::shared_ptr<google::protobuf::MessageLite> msg);

private:
    std::unique_ptr<JvmClientProxy> jvm_client_;
    std::string auth_token_;
    std::string p2p_username_;
    std::string p2p_password_;
    VideoDecodeRenderPipeline::Params video_params_;
    AudioPlayer::Params audio_params_{};
    std::vector<std::string> reflex_servers_;
    std::mutex dr_mutex_;
    std::unique_ptr<VideoDecodeRenderPipeline> video_pipeline_;
    std::unique_ptr<AudioPlayer> audio_player_;
    lt::tp::Client* tp_client_ = nullptr;
    std::unique_ptr<ltlib::TaskThread> thread_;
    bool should_exit_ = true;
    ltlib::TimeSync time_sync_;
    int64_t rtt_ = 0;
    int64_t time_diff_ = 0;
    std::optional<bool> is_p2p_;
    bool absolute_mouse_ = true;
    bool last_w_or_h_is_0_ = false;
    int64_t last_received_keepalive_ = 0;
};

} // namespace lt
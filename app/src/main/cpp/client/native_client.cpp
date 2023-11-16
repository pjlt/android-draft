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

#include <ltproto/client2app/client_status.pb.h>
#include <ltproto/client2service/time_sync.pb.h>
#include <ltproto/client2worker/cursor_info.pb.h>
#include <ltproto/client2worker/request_keyframe.pb.h>
#include <ltproto/client2worker/send_side_stat.pb.h>
#include <ltproto/client2worker/start_transmission.pb.h>
#include <ltproto/client2worker/start_transmission_ack.pb.h>
#include <ltproto/client2worker/switch_mouse_mode.pb.h>
#include <ltproto/common/keep_alive.pb.h>
#include <ltproto/server/request_connection.pb.h>
#include <ltproto/server/request_connection_ack.pb.h>
#include <ltproto/signaling/join_room.pb.h>
#include <ltproto/signaling/join_room_ack.pb.h>
#include <ltproto/signaling/signaling_message.pb.h>
#include <ltproto/signaling/signaling_message_ack.pb.h>

#include <ltlib/logging.h>
#include <ltproto/ltproto.h>

#include <rtc/rtc.h>

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

std::string toString(lt::VideoCodecType codec) {
    switch (codec) {
    case lt::VideoCodecType::H264:
        return "AVC";
    case lt::VideoCodecType::H265:
        return "HEVC";
    case lt::VideoCodecType::Unknown:
    default:
        return "?";
    }
}

} // namespace

namespace lt {

bool LtNativeClient::Params::validate() const {
    if (jvm_client == nullptr) {
        return false;
    }
    if (client_id.empty() || room_id.empty() || token.empty() || p2p_username.empty() ||
        p2p_password.empty() || signaling_address.empty()) {
        LOG(ERR) << "LtNativeClient::Params some string are empty";
        return false;
    }
    if (signaling_port <= 0 || signaling_port > 65535) {
        LOG(ERR) << "LtNativeClient::Params invalid signaling port";
        return false;
    }
    if (codec != "avc" && codec != "hevc") {
        LOG(ERR) << "LtNativeClient::Params invalid codec: " << codec;
        return false;
    }
    if (audio_channels <= 0 || audio_freq <= 0) {
        LOGF(ERR, "LtNativeClient::Params invalid audio params{channels:%d, freq:%d}",
             audio_channels, audio_freq);
        return false;
    }
    if (width == 0 || height == 0) {
        LOGF(ERR, "LtNativeClient::Params invalid video size{w:%d, h:%d}", width, height);
        return false;
    }
    return true;
}

LtNativeClient* LtNativeClient::create(const LtNativeClient::Params& params) {
    auto proxy = JvmClientProxy::create(params.jvm_client);
    if (proxy == nullptr) {
        return nullptr;
    }
    auto cli = new LtNativeClient{params};
    cli->jvm_client_ = std::move(proxy);
    return cli;
}

void LtNativeClient::destroy(LtNativeClient* obj) {
    delete obj;
}

LtNativeClient::LtNativeClient(const Params& params)
    : auth_token_{params.token}
    , p2p_username_{params.p2p_username}
    , p2p_password_{params.p2p_password}
    , video_params_{to_ltrtc(params.codec),
                    params.width,
                    params.height,
                    params.screen_refresh_rate,
                    params.video_surface,
                    std::bind(&LtNativeClient::sendMessageToHost, this, std::placeholders::_1,
                              std::placeholders::_2, std::placeholders::_3)}
    , audio_params_{AudioCodecType::PCM, static_cast<uint32_t>(params.audio_freq),
                    static_cast<uint32_t>(params.audio_channels)}
    , reflex_servers_{params.reflex_servers} {}

LtNativeClient::~LtNativeClient() {
    // LtNativeClient和lanthing-pc的Client的线程模型是不一样的，析构要小心处理
}

bool LtNativeClient::start() {
    thread_ = ltlib::TaskThread::create("native_client");
    if (!initTransport()) {
        LOG(INFO) << "Initialize rtc failed";
        return false;
    }
    LOG(INFO) << "Initialize rtc success";
    return true;
}

void LtNativeClient::postTask(const std::function<void()>& task) {
    thread_->post(task);
}

void LtNativeClient::postDelayTask(int64_t delay_ms, const std::function<void()>& task) {
    thread_->post_delay(ltlib::TimeDelta{delay_ms * 1000}, task);
}

void LtNativeClient::checkWorkerTimeout() {
    constexpr int64_t kFiveSeconds = 5'000;
    constexpr int64_t k500ms = 500;
    auto now = ltlib::steady_now_ms();
    if (now - last_received_keepalive_ > kFiveSeconds) {
        LOG(INFO) << "Didn't receive KeepAliveAck from worker for "
                  << (now - last_received_keepalive_) << "ms, exit";
        tellAppKeepAliveTimeout();
        // 为了让消息发送到app，延迟50ms再关闭程序
        postDelayTask(50, [this]() { jvm_client_->onNativeClosed(); });
        return;
    }
    postDelayTask(k500ms, std::bind(&LtNativeClient::checkWorkerTimeout, this));
}

void LtNativeClient::syncTime() {
    auto msg = std::make_shared<ltproto::client2service::TimeSync>();
    msg->set_t0(time_sync_.getT0());
    msg->set_t1(time_sync_.getT1());
    msg->set_t2(ltlib::steady_now_us());
    sendMessageToHost(ltproto::id(msg), msg, true);
    constexpr uint32_t k500ms = 500;
    postDelayTask(k500ms, std::bind(&LtNativeClient::syncTime, this));
}

void LtNativeClient::switchMouseMode() {
    absolute_mouse_ = !absolute_mouse_;
    video_pipeline_->switchMouseMode(absolute_mouse_);
    auto msg = std::make_shared<ltproto::client2worker::SwitchMouseMode>();
    msg->set_absolute(absolute_mouse_);
    sendMessageToHost(ltproto::id(msg), msg, true);
}

void LtNativeClient::tellAppKeepAliveTimeout() {
    // FIXME: 具体通知是KeepAliveTimeout引起的断链
    jvm_client_->onNativeClosed();
}

bool LtNativeClient::initTransport() {
    rtc::Client::Params params{};
    params.user_data = this;
    params.use_nbp2p = true;
    std::vector<const char*> reflex_servers;
    for (auto& svr : reflex_servers_) {
        reflex_servers.push_back(svr.data());
        // LOG(DEBUG) << "Reflex: " << svr;
    }
    if (params.use_nbp2p) {
        params.nbp2p_params.disable_ipv6 = false;
        params.nbp2p_params.disable_lan_udp = false;
        params.nbp2p_params.disable_mapping = false;
        params.nbp2p_params.disable_reflex = false;
        params.nbp2p_params.disable_relay = false;
        params.nbp2p_params.username = p2p_username_.c_str();
        params.nbp2p_params.password = p2p_password_.c_str();
        params.nbp2p_params.reflex_servers = reflex_servers.data();
        params.nbp2p_params.reflex_servers_count = static_cast<uint32_t>(reflex_servers.size());
        params.nbp2p_params.relay_servers = nullptr;
        params.nbp2p_params.relay_servers_count = 0;
    }
    params.on_data = &LtNativeClient::onTpData;
    params.on_video = &LtNativeClient::onTpVideoFrame;
    params.on_audio = &LtNativeClient::onTpAudioData;
    params.on_connected = &LtNativeClient::onTpConnected;
    params.on_conn_changed = &LtNativeClient::onTpConnChanged;
    params.on_failed = &LtNativeClient::onTpFailed;
    params.on_disconnected = &LtNativeClient::onTpDisconnected;
    params.on_signaling_message = &LtNativeClient::onTpSignalingMessage;
    params.video_codec_type = video_params_.codec_type;
    params.audio_channels = audio_params_.channels;
    params.audio_sample_rate = audio_params_.frames_per_second;
    tp_client_ = rtc::Client::create(params);
    if (tp_client_ == nullptr) {
        LOG(ERR) << "Create lt::tp::Client failed";
        return false;
    }

    if (!tp_client_->connect()) {
        LOG(INFO) << "lt::tp::Client connect failed";
        return false;
    }
    return true;
}

void LtNativeClient::onTpData(void* user_data, const uint8_t* data, uint32_t size,
                              bool is_reliable) {
    auto that = reinterpret_cast<LtNativeClient*>(user_data);
    (void)is_reliable;
    auto type = reinterpret_cast<const uint32_t*>(data);
    auto msg = ltproto::create_by_type(*type);
    if (msg == nullptr) {
        LOG(INFO) << "Unknown message type: " << *type;
    }
    bool success = msg->ParseFromArray(data + 4, size - 4);
    if (!success) {
        LOG(INFO) << "Parse message failed, type: " << *type;
        return;
    }
    that->dispatchRemoteMessage(*type, msg);
}

void LtNativeClient::onTpVideoFrame(void* user_data, const lt::VideoFrame& frame) {
    auto that = reinterpret_cast<LtNativeClient*>(user_data);
    std::lock_guard lock{that->dr_mutex_};
    if (that->video_pipeline_ == nullptr) {
        return;
    }
    VideoDecodeRenderPipeline::Action action = that->video_pipeline_->submit(frame);
    switch (action) {
    case VideoDecodeRenderPipeline::Action::REQUEST_KEY_FRAME:
    {
        auto req = std::make_shared<ltproto::client2worker::RequestKeyframe>();
        that->sendMessageToHost(ltproto::id(req), req, true);
        break;
    }
    case VideoDecodeRenderPipeline::Action::NONE:
        break;
    default:
        break;
    }
}

void LtNativeClient::onTpAudioData(void* user_data, const lt::AudioData& audio_data) {
    auto that = reinterpret_cast<LtNativeClient*>(user_data);
    // FIXME: transport在audio_player_实例化前，不应回调audio数据
    if (that->audio_player_) {
        that->audio_player_->submit(audio_data.data, audio_data.size);
    }
}

void LtNativeClient::onTpConnected(void* user_data, lt::LinkType link_type) {
    auto that = reinterpret_cast<LtNativeClient*>(user_data);
    (void)link_type;
    that->video_pipeline_ = VideoDecodeRenderPipeline::create(that->video_params_);
    if (that->video_pipeline_ == nullptr) {
        LOG(ERR) << "Create VideoDecodeRenderPipeline failed";
        return;
    }
    that->audio_player_ = AudioPlayer::create(that->audio_params_);
    if (that->audio_player_ == nullptr) {
        LOG(INFO) << "Create AudioPlayer failed";
        return;
    }
    // 心跳检测
    that->thread_->post(std::bind(&LtNativeClient::sendKeepAlive, that));
    that->last_received_keepalive_ = ltlib::steady_now_ms();
    that->postDelayTask(500, std::bind(&LtNativeClient::checkWorkerTimeout, that));
    // 如果未来有“串流”以外的业务，在这个StartTransmission添加字段.
    auto start = std::make_shared<ltproto::client2worker::StartTransmission>();
    start->set_client_os(ltproto::client2worker::StartTransmission_ClientOS_Windows);
    start->set_token(that->auth_token_);
    that->sendMessageToHost(ltproto::id(start), start, true);
    that->postTask(std::bind(&LtNativeClient::syncTime, that));

    // setTitle
    that->is_p2p_ = link_type != lt::LinkType::RelayUDP;
    std::ostringstream oss;
    oss << "Lanthing " << (that->is_p2p_.value() ? "P2P " : "Relay ")
        << toString(that->video_params_.codec_type) << " GPU:GPU"; // 暂时只支持硬件编解码.
    // that->sdl_->setTitle(oss.str());
    that->jvm_client_->onNativeConnected();
}

void LtNativeClient::onTpConnChanged(void*) {}

void LtNativeClient::onTpFailed(void* user_data) {
    auto that = reinterpret_cast<LtNativeClient*>(user_data);
    // that->stopWait();
    that->jvm_client_->onNativeClosed();
}

void LtNativeClient::onTpDisconnected(void* user_data) {
    auto that = reinterpret_cast<LtNativeClient*>(user_data);
    // that->stopWait();
    that->jvm_client_->onNativeClosed();
}

void LtNativeClient::onTpSignalingMessage(void* user_data, const char* key, const char* value) {
    auto that = reinterpret_cast<LtNativeClient*>(user_data);
    // 将key和value封装在proto里.
    //    auto msg = std::make_shared<ltproto::signaling::SignalingMessage>();
    //    msg->set_level(ltproto::signaling::SignalingMessage::Rtc);
    //    auto rtc_msg = msg->mutable_rtc_message();
    //    rtc_msg->set_key(key);
    //    rtc_msg->set_value(value);
    // that->postTask([that, msg]() { that->signaling_client_->send(ltproto::id(msg), msg); });
    std::string _key = key;
    std::string _value = value;
    that->postTask(
        [that, _key, _value] { that->jvm_client_->onNativeSignalingMessage(_key, _value); });
}

void LtNativeClient::dispatchRemoteMessage(
    uint32_t type, const std::shared_ptr<google::protobuf::MessageLite>& msg) {
    switch (type) {
    case ltproto::type::kKeepAliveAck:
        onKeepAliveAck();
        break;
    case ltproto::type::kStartTransmissionAck:
        onStartTransmissionAck(msg);
        break;
    case ltproto::type::kTimeSync:
        onTimeSync(msg);
        break;
    case ltproto::type::kSendSideStat:
        onSendSideStat(msg);
        break;
    case ltproto::type::kCursorInfo:
        onCursorInfo(msg);
        break;
    default:
        LOG(WARNING) << "Unknown message type: " << type;
        break;
    }
}

void LtNativeClient::sendKeepAlive() {
    auto keep_alive = std::make_shared<ltproto::common::KeepAlive>();
    sendMessageToHost(ltproto::id(keep_alive), keep_alive, true);

    const auto k500ms = ltlib::TimeDelta{500'000};
    thread_->post_delay(k500ms, std::bind(&LtNativeClient::sendKeepAlive, this));
}

void LtNativeClient::onKeepAliveAck() {
    // Ack是worker回复的，其它消息可能是service发送的，我们的目的是判断worker还在不在，所以只用KeepAliveAck来更新时间
    last_received_keepalive_ = ltlib::steady_now_ms();
}

bool LtNativeClient::sendMessageToHost(uint32_t type,
                                       const std::shared_ptr<google::protobuf::MessageLite>& msg,
                                       bool reliable) {
    auto packet = ltproto::Packet::create({type, msg}, false);
    if (!packet.has_value()) {
        LOG(ERR) << "Create ltproto::Packet failed, type:" << type;
        return false;
    }
    const auto& pkt = packet.value();
    // WebRTC的数据通道可以帮助我们完成stream->packet的过程，所以这里不需要把packet
    // header一起传过去.
    bool success = tp_client_->sendData(pkt.payload.get(), pkt.header.payload_size, reliable);
    return success;
}

void LtNativeClient::onStartTransmissionAck(
    const std::shared_ptr<google::protobuf::MessageLite>& _msg) {
    auto msg = std::static_pointer_cast<ltproto::client2worker::StartTransmissionAck>(_msg);
    if (msg->err_code() == ltproto::ErrorCode::Success) {
        LOG(INFO) << "Received StartTransmissionAck with success";
    }
    else {
        LOG(INFO) << "StartTransmission failed with " << ltproto::ErrorCode_Name(msg->err_code());
        // stopWait();
        jvm_client_->onNativeClosed();
    }
}

void LtNativeClient::onTimeSync(std::shared_ptr<google::protobuf::MessageLite> _msg) {
    auto msg = std::static_pointer_cast<ltproto::client2service::TimeSync>(_msg);
    auto result = time_sync_.calc(msg->t0(), msg->t1(), msg->t2(), ltlib::steady_now_us());
    if (result.has_value()) {
        rtt_ = result->rtt;
        time_diff_ = result->time_diff;
        LOG(DEBUG) << "rtt:" << rtt_ << ", time_diff:" << time_diff_;
        if (video_pipeline_) {
            video_pipeline_->setTimeDiff(time_diff_);
            video_pipeline_->setRTT(rtt_);
        }
    }
}

void LtNativeClient::onSendSideStat(std::shared_ptr<google::protobuf::MessageLite> _msg) {
    auto msg = std::static_pointer_cast<ltproto::client2worker::SendSideStat>(_msg);
    video_pipeline_->setNack(static_cast<uint32_t>(msg->nack()));
    video_pipeline_->setBWE(static_cast<uint32_t>(msg->bwe()));
}

void LtNativeClient::onCursorInfo(std::shared_ptr<google::protobuf::MessageLite> _msg) {
    auto msg = std::static_pointer_cast<ltproto::client2worker::CursorInfo>(_msg);
    LOGF(DEBUG, "onCursorInfo id:%d, w:%d, h:%d, x:%d, y%d", msg->preset(), msg->w(), msg->h(),
         msg->x(), msg->y());
    if (msg->w() == 0 || msg->h() == 0) {
        // 这个这么丑的flag，只是为了不让这行错误日志频繁打
        if (!last_w_or_h_is_0_) {
            last_w_or_h_is_0_ = true;
            LOG(ERR) << "Received CursorInfo with w " << msg->w() << " h " << msg->h();
        }
        return;
    }
    last_w_or_h_is_0_ = false;
    video_pipeline_->setCursorInfo(msg->preset(), 1.0f * msg->x() / msg->w(),
                                   1.0f * msg->y() / msg->h(), msg->visible());
}

void LtNativeClient::onPlatformStop() {
    //???
}

void LtNativeClient::onSignalingMessage(const std::string& key, const std::string& value) {
    tp_client_->onSignalingMessage(key.c_str(), value.c_str());
}

} // namespace lt
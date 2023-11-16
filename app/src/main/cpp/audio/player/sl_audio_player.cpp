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

#include "sl_audio_player.h"

#include <ltlib/logging.h>

// 吐槽：SLES的接口也太微软了吧😱

namespace lt {

SLAudioPlayer::SLAudioPlayer(const Params& params)
    : AudioPlayer{params} {}

SLAudioPlayer::~SLAudioPlayer() {
    std::lock_guard<std::mutex> lock{mutex_};
    if (player_ != nullptr) {
        (*player_)->SetPlayState(player_, SL_PLAYSTATE_STOPPED);
        player_ = nullptr;
    }
    if (queue_ != nullptr) {
        (*queue_)->Clear(queue_);
        queue_ = nullptr;
    }
    if (player_obj_ != nullptr) {
        (*player_obj_)->Destroy(player_obj_);
        player_obj_ = nullptr;
    }
    if (mixer_ != nullptr) {
        (*mixer_)->Resume(mixer_, SL_BOOLEAN_FALSE);
        (*mixer_)->Destroy(mixer_);
        mixer_ = nullptr;
    }
    engine_ = nullptr;
    if (engine_obj_ != nullptr) {
        (*engine_obj_)->Destroy(engine_obj_);
        engine_obj_ = nullptr;
    }
}

bool SLAudioPlayer::initPlatform() {
    SLresult res = slCreateEngine(&engine_obj_, 0, nullptr, 0, nullptr, nullptr);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slCreateEngine failed %d", res);
        return false;
    }
    res = (*engine_obj_)->Realize(engine_obj_, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slEngineObj::Realize failed %d", res);
        return false;
    }
    res = (*engine_obj_)->GetInterface(engine_obj_, SL_IID_ENGINE, &engine_);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slEngineObj::GetInterface(SL_IID_ENGINE) failed %d", res);
        return false;
    }
    res = (*engine_)->CreateOutputMix(engine_, &mixer_, 0, nullptr, nullptr);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slEngine::CreateOutputMix failed %d", res);
        return false;
    }
    res = (*mixer_)->Realize(mixer_, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slMixer::Realize failed %d", res);
        return false;
    }
    SLDataLocator_OutputMix outputmix = {SL_DATALOCATOR_OUTPUTMIX, mixer_};
    SLDataSink sink = {&outputmix, 0};
    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,
                            channels(),
                            framesPerSec() * 1000,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource ds = {&que, &pcm};
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    res = (*engine_)->CreateAudioPlayer(engine_, &player_obj_, &ds, &sink, //
                                        sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slEngine::CreateAudioPlayer failed %d", res);
        return false;
    }
    res = (*player_obj_)->Realize(player_obj_, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slPlayerObj::Realize failed %d", res);
        return false;
    }
    res = (*player_obj_)->GetInterface(player_obj_, SL_IID_PLAY, &player_);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slPlayerObj::GetInterface(SL_IID_PLAY) failed %d", res);
        return false;
    }
    res = (*player_obj_)->GetInterface(player_obj_, SL_IID_BUFFERQUEUE, &queue_);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slPlayerObj::GetInterface(SL_IID_BUFFERQUEUE) failed %d", res);
        return false;
    }
    res = (*queue_)->RegisterCallback(queue_, slCallback, this);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slQueue::RegisterCallback() failed %d", res);
        return false;
    }
    res = (*player_)->SetPlayState(player_, SL_PLAYSTATE_PLAYING);
    if (res != SL_RESULT_SUCCESS) {
        LOGF(ERR, "slPlayer::SetPlayState(SL_PLAYSTATE_PLAYING) failed %d", res);
        return false;
    }
    (*queue_)->Enqueue(queue_, "", 1);
    return true;
}

bool SLAudioPlayer::play(const void* _data, uint32_t size) {
    {
        std::lock_guard<std::mutex> lock{mutex_};
        auto data = reinterpret_cast<const uint8_t*>(_data);
        buffer_.push_back(std::vector<uint8_t>(data, data + size));
    }
    return true;
}

void SLAudioPlayer::slCallback(SLAndroidSimpleBufferQueueItf bq, void* context) {
    auto that = reinterpret_cast<SLAudioPlayer*>(context);
    that->doPlay();
}

void SLAudioPlayer::doPlay() {
    std::lock_guard<std::mutex> lock{mutex_};
    while (!buffer_.empty()) {
        (*queue_)->Enqueue(queue_, buffer_.back().data(), buffer_.back().size());
        buffer_.pop_back();
    }
}

} // namespace lt
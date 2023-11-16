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
#include <audio/player/audio_player.h>

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace lt {

class SLAudioPlayer : public AudioPlayer {
public:
    SLAudioPlayer(const Params& params);
    ~SLAudioPlayer() override;
    bool initPlatform() override;
    bool play(const void* data, uint32_t size) override;

private:
    static void slCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
    void doPlay();

private:
    SLObjectItf engine_obj_ = nullptr;
    SLEngineItf engine_ = nullptr;
    SLObjectItf player_obj_ = nullptr;
    SLPlayItf player_ = nullptr;
    SLObjectItf mixer_ = nullptr;
    SLAndroidSimpleBufferQueueItf queue_ = nullptr;
    std::deque<std::vector<uint8_t>> buffer_;
    std::mutex mutex_;
    std::vector<uint8_t> dummy_audio_;

};

} // namespace lt
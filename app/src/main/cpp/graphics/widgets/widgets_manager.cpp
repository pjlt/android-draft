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

#include "widgets_manager.h"

namespace lt {

std::unique_ptr<WidgetsManager> WidgetsManager::create(const WidgetsManager::Params& params) {
    return std::unique_ptr<WidgetsManager>(new WidgetsManager{params});
}

WidgetsManager::~WidgetsManager() {}

void WidgetsManager::render() {}

void WidgetsManager::reset() {}

void WidgetsManager::enableStatus() {}

void WidgetsManager::disableStatus() {}
void WidgetsManager::enableStatistics() {}
void WidgetsManager::disableStatistics() {}
void WidgetsManager::setTaskBarPos(uint32_t direction, uint32_t left, uint32_t right, uint32_t top,
                                   uint32_t bottom) {
    (void)direction;
    (void)left;
    (void)right;
    (void)top;
    (void)bottom;
}
void WidgetsManager::updateStatus(uint32_t rtt_ms, uint32_t fps, float loss) {
    (void)rtt_ms;
    (void)fps;
    (void)loss;
}

void WidgetsManager::updateStatistics(const VideoStatistics::Stat& statistics) {
    (void)statistics;
}

WidgetsManager::WidgetsManager(const WidgetsManager::Params& params) {
    (void)params;
}
} // namespace lt
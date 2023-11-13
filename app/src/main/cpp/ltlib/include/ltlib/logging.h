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

#include <cinttypes>
#include <sstream>

#include <android/log.h>

#define DEBUG ANDROID_LOG_DEBUG
#define INFO ANDROID_LOG_INFO
#define WARNING ANDROID_LOG_WARN
#define ERR ANDROID_LOG_ERROR
#define FATAL ANDROID_LOG_FATAL

namespace ltlib {

bool logLevel(const android_LogPriority& level);
void disableLogLevel(const android_LogPriority& level);
void enableLogLevel(const android_LogPriority& level);

struct LogCapture {
    LogCapture(const char* file, const int line, const char* function,
               const android_LogPriority& level);
    ~LogCapture();
    std::ostringstream &stream() {
        return _stream;
    }
private:
    std::ostringstream _stream;
    const char* _file;
    const int _line;
    const char* _function;
    const android_LogPriority _level;
};

} // namespace ltlib

#define LOGF(level, ...) __android_log_print(level, "ltmsdk", __VA_ARGS__)

#if defined(_MSC_VER) && (defined(WINDOWS_FUNCSIG)) // Microsoft
#define G3LOG_PRETTY_FUNCTION __FUNCSIG__
#elif defined(__GNUC__) && defined(PRETTY_FUNCTION) // GCC compatible
#define G3LOG_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define G3LOG_PRETTY_FUNCTION __FUNCTION__
#endif

#define INTERNAL_LOG_MESSAGE(level)                                                                \
    ltlib::LogCapture(__FILE__, __LINE__, static_cast<const char*>(G3LOG_PRETTY_FUNCTION), level)

#define LOG(level)                                                                                 \
    if (!ltlib::logLevel(level)) {                                                                        \
    }                                                                                              \
    else                                                                                           \
        INTERNAL_LOG_MESSAGE(level).stream()
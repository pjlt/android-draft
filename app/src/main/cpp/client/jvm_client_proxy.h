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
#include <memory>

#include <jni.h>

namespace lt {

class JvmClientProxy {
public:
    struct JMethodInfo {
        JMethodInfo(const char* _name, const char* _signature)
                : name{_name}
                , signature{_signature} {}
        const char* name;
        const char* signature;
    };

public:
    static std::unique_ptr<JvmClientProxy> create(int64_t jvm_obj);
    ~JvmClientProxy();
    JvmClientProxy(const JvmClientProxy&) = delete;
    JvmClientProxy(JvmClientProxy&&) = delete;
    JvmClientProxy& operator=(const JvmClientProxy&) = delete;
    JvmClientProxy& operator=(JvmClientProxy&&) = delete;

    void onNativeClosed(/*reason*/);
    void onNativeSignalingMessage(const std::string& key, const std::string& value);
    void onNativeConnected();

private:
    JvmClientProxy(int64_t jvm_obj);
    bool init();
    bool loadMethod(JNIEnv* env, JMethodInfo info, jmethodID& jmid);

private:
    jobject obj_;
    jclass class_ = nullptr;
    jmethodID on_closed_ = nullptr;
    jmethodID on_signaling_message_ = nullptr;
    jmethodID on_connected_ = nullptr;
};

} // namespace lt
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

#include "jvm_client_proxy.h"

#include <ltlib/logging.h>

extern JavaVM* g_jvm;

namespace {

struct JMethods {
    lt::JvmClientProxy::JMethodInfo onNativeClosed = {"onNativeClosed", "()V"};
    lt::JvmClientProxy::JMethodInfo onNativeSignalingMessage = {"onNativeSignalingMessage",
                                                                "(Ljava/lang/String;[B)V"};
    lt::JvmClientProxy::JMethodInfo onNativeConnected = {"onNativeConnected", "()V"};
};

const char* kJvmClientClassName = "cn/lanthing/ltmsdk/LtClient";

} // namespace

namespace lt {

std::unique_ptr<JvmClientProxy> JvmClientProxy::create(jobject jvm_obj) {
    std::unique_ptr<JvmClientProxy> proxy{new JvmClientProxy{jvm_obj}};
    if (!proxy->init()) {
        return nullptr;
    }
    return proxy;
}

JvmClientProxy::JvmClientProxy(jobject jvm_obj)
    : obj_{reinterpret_cast<jobject>(jvm_obj)} {}

JvmClientProxy::~JvmClientProxy() {
    // 得到的obj_似乎不需要处理
}

bool JvmClientProxy::init() {
    JNIEnv* env = nullptr;
    const char* kClientClosed = "";
    g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    class_ = env->FindClass(kJvmClientClassName);
    if (class_ == nullptr) {
        LOG(ERR) << "FindClass '" << kJvmClientClassName << "' failed";
        return false;
    }
    JMethods methods;
    if (!loadMethod(env, methods.onNativeClosed, on_closed_) ||
        !loadMethod(env, methods.onNativeConnected, on_connected_) ||
        !loadMethod(env, methods.onNativeSignalingMessage, on_signaling_message_)) {
        return false;
    }
    return true;
}

void JvmClientProxy::onNativeClosed() {
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    env->CallVoidMethod(obj_, on_closed_);
    g_jvm->DetachCurrentThread();
}

void JvmClientProxy::onNativeSignalingMessage(const std::string& key, const std::string& value) {
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    jbyteArray jb = env->NewByteArray(static_cast<jsize>(value.size()));
    env->SetByteArrayRegion(jb, 0, static_cast<jsize>(value.size()), reinterpret_cast<const jbyte*>(value.data()));
    env->CallVoidMethod(obj_, on_signaling_message_, env->NewStringUTF(key.c_str()), jb);
    g_jvm->DetachCurrentThread();
}

void JvmClientProxy::onNativeConnected() {
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    env->CallVoidMethod(obj_, on_connected_);
    g_jvm->DetachCurrentThread();
}

bool JvmClientProxy::loadMethod(JNIEnv* env, JMethodInfo info, jmethodID& jmid) {
    jmid = env->GetMethodID(class_, info.name, info.signature);
    if (jmid == nullptr) {
        LOG(ERR) << "GetMethodID(" << info.name << ") failed";
        return false;
    }
    return true;
}

} // namespace lt
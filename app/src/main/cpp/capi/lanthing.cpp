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

#include <cstdint>
#include <string>
#include <vector>

#include <jni.h>

#include <client/native_client.h>

namespace {
    std::string jStr2Std(JNIEnv *env, jstring jstr) {
        jsize length = env->GetStringLength(jstr);
        const char* str = env->GetStringUTFChars((jstring)jstr, nullptr);
        auto string = std::string(str, length);
        env->ReleaseStringUTFChars(jstr, str);
        return string;
    }
    LtNativeClient* ncast(jlong ptr) {
        return reinterpret_cast<LtNativeClient*>(ptr);
    }
} // namespace

extern "C"
JNIEXPORT jlong JNICALL
Java_cn_lanthing_ltmsdk_LtClient_createNativeClient(JNIEnv *env, jobject thiz, jstring client_id,
                                                    jstring room_id, jstring token,
                                                    jstring p2p_username, jstring p2p_password,
                                                    jstring signaling_address, jint signaling_port,
                                                    jstring codec_type, jint audio_channels,
                                                    jint audio_freq, jobject reflex_servers) {
    jclass cList = env->FindClass("java/util/List");
    jmethodID mSize = env->GetMethodID(cList, "size", "()I");
    jmethodID mGet = env->GetMethodID(cList, "get", "(I)Ljava/lang/Object;");
    if (mSize == nullptr || mGet == nullptr) {
        return -1;
    }
    jint svrSize = env->CallIntMethod(reflex_servers, mSize);
    std::vector<std::string> rflxs;
    for (jint i = 0; i < svrSize; i++) {
        auto strObj = (jstring)env->CallObjectMethod(reflex_servers, mGet, i);
        rflxs.push_back(jStr2Std(env, strObj));
    }
    LtNativeClient::Params params{};
    params.client_id = jStr2Std(env, client_id);
    params.room_id = jStr2Std(env, room_id);
    params.token = jStr2Std(env, token);
    params.p2p_username = jStr2Std(env, p2p_username);
    params.p2p_password = jStr2Std(env, p2p_password);
    params.signaling_address = jStr2Std(env, signaling_address);
    params.signaling_port = signaling_port;
    params.codec = jStr2Std(env, codec_type);
    params.audio_channels = audio_channels;
    params.audio_freq = audio_freq;
    params.reflex_servers = rflxs;
    if (!params.validate()) {
        return -1;
    }
    return reinterpret_cast<jlong>(new LtNativeClient{params});
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_lanthing_ltmsdk_LtClient_destroyNativeClient(JNIEnv *env, jobject thiz, jlong cli) {
    auto obj = reinterpret_cast<LtNativeClient*>(cli);
    delete obj;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_cn_lanthing_ltmsdk_LtClient_nativeStart(JNIEnv *env, jobject thiz, jlong cli) {
    return ncast(cli)->start();
}
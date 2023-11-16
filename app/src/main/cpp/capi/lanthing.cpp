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
#include <ltlib/logging.h>
#include <ltlib/threads.h>

JavaVM* g_jvm = nullptr;

namespace {
std::string jStr2Std(JNIEnv* env, jstring jstr) {
    jsize length = env->GetStringLength(jstr);
    const char* str = env->GetStringUTFChars((jstring)jstr, nullptr);
    auto string = std::string(str, length);
    env->ReleaseStringUTFChars(jstr, str);
    return string;
}
lt::LtNativeClient* ncast(jlong ptr) {
    return reinterpret_cast<lt::LtNativeClient*>(ptr);
}
} // namespace

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOG(INFO) << "JNI_OnLoad";
    (void)reserved;
    g_jvm = vm;
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jlong JNICALL Java_cn_lanthing_ltmsdk_LtClient_createNativeClient(
    JNIEnv* env, jobject thiz, jobject video_surface, jobject cursor_surface,
    jint videoWidth, jint videoHeight, jstring client_id,
    jstring room_id, jstring token, jstring p2p_username, jstring p2p_password,
    jstring signaling_address, jint signaling_port, jstring codec_type, jint audio_channels,
    jint audio_freq, jobject reflex_servers) {

    LOG(INFO) << "createNativeClient JvmClient " << thiz;
    ltlib::ThreadWatcher::instance()->disableCrashOnTimeout();
    jclass cList = env->FindClass("java/util/List");
    jmethodID mSize = env->GetMethodID(cList, "size", "()I");
    jmethodID mGet = env->GetMethodID(cList, "get", "(I)Ljava/lang/Object;");
    if (mSize == nullptr || mGet == nullptr) {
        return 0;
    }
    jint svrSize = env->CallIntMethod(reflex_servers, mSize);
    std::vector<std::string> rflxs;
    for (jint i = 0; i < svrSize; i++) {
        auto strObj = (jstring)env->CallObjectMethod(reflex_servers, mGet, i);
        rflxs.push_back(jStr2Std(env, strObj));
    }
    lt::LtNativeClient::Params params{};
    params.jvm_client = env->NewGlobalRef(thiz); // NOTE: 由内部释放
    params.video_surface = env->NewGlobalRef(video_surface);
    params.cursor_surface = env->NewGlobalRef(cursor_surface);
    params.width = static_cast<uint32_t>(std::max(0, videoWidth));
    params.height = static_cast<uint32_t>(std::max(0, videoHeight));
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
        return 0;
    }
    auto cli = lt::LtNativeClient::create(params);
    if (cli != nullptr) {
        return reinterpret_cast<jlong>(cli);
    }
    else {
        // reinterpret_cast<jlong>(nullptr) -> ???
        return 0;
    }
}

extern "C" JNIEXPORT void JNICALL Java_cn_lanthing_ltmsdk_LtClient_destroyNativeClient(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jlong cli) {
    lt::LtNativeClient::destroy(ncast(cli));
}

extern "C" JNIEXPORT jboolean JNICALL Java_cn_lanthing_ltmsdk_LtClient_nativeStart(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong cli) {
    return ncast(cli)->start();
}

extern "C" JNIEXPORT void JNICALL
Java_cn_lanthing_ltmsdk_LtClient_nativeSwitchMouseMode(JNIEnv* env, jobject thiz, jlong cli) {
    ncast(cli)->switchMouseMode();
}

extern "C" JNIEXPORT void JNICALL Java_cn_lanthing_ltmsdk_LtClient_nativeStop(JNIEnv* env,
                                                                              jobject thiz,
                                                                              jlong cli) {
    ncast(cli)->onPlatformStop();
}

extern "C" JNIEXPORT void JNICALL Java_cn_lanthing_ltmsdk_LtClient_nativeOnSignalingMessage(
    JNIEnv* env, jobject thiz, jlong cli, jstring key, jbyteArray value) {
    jboolean isCopy;
    jbyte* jb = env->GetByteArrayElements(value, &isCopy);
    jsize size = env->GetArrayLength(value);
    std::string real_value(jb, jb + size);
    env->ReleaseByteArrayElements(value, jb, 0);
    ncast(cli)->onSignalingMessage(jStr2Std(env, key), real_value);
}
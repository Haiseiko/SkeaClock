#include <jni.h>
#include <chrono>
#include <ctime>
#include <android/log.h>
#include <android/native_window_jni.h>

#include "vulkan_renderer.h"

#define LOG_TAG "SkeaClockNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static VulkanRenderer g_renderer;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_haiseiko_skeaclock_MainActivity_nativeIsVulkanAvailable(JNIEnv* env, jobject /*thiz*/) {
    return static_cast<jboolean>(VulkanRenderer::isSupported());
}

extern "C" JNIEXPORT void JNICALL
Java_com_haiseiko_skeaclock_MainActivity_nativeInitVulkan(JNIEnv* env, jobject /*thiz*/, jobject surface) {
    if (surface == nullptr) {
        LOGI("Native init: null surface");
        return;
    }

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        LOGI("Native init: ANativeWindow creation failed");
        return;
    }

    if (!g_renderer.init(window)) {
        LOGI("Native init: Vulkan init failed");
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_haiseiko_skeaclock_MainActivity_nativeRenderFrame(JNIEnv* env, jobject /*thiz*/) {
    g_renderer.renderFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_haiseiko_skeaclock_MainActivity_nativeShutdownVulkan(JNIEnv* env, jobject /*thiz*/) {
    g_renderer.destroy();
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_haiseiko_skeaclock_SkeaClockNative_getTimeAngles(JNIEnv* env, jclass /*clazz*/) {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    std::time_t t = static_cast<std::time_t>(ms / 1000);
    struct tm buf;
#if defined(_WIN32)
    localtime_s(&buf, &t);
#else
    localtime_r(&t, &buf);
#endif
    int hour = buf.tm_hour;
    int minute = buf.tm_min;
    int second = buf.tm_sec;
    float secondTurn = (second + (ms % 1000) / 1000.0f) / 60.0f;
    float minuteTurn = (minute + secondTurn) / 60.0f;
    float hourTurn = (hour % 12 + minuteTurn) / 12.0f;

    jfloat vals[3] = { hourTurn, minuteTurn, secondTurn };
    jfloatArray arr = env->NewFloatArray(3);
    env->SetFloatArrayRegion(arr, 0, 3, vals);
    return arr;
}

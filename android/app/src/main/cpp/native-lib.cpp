#include <jni.h>
#include <chrono>
#include <ctime>

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_haiseiko_skeaclock_SkeaClockNative_getTimeAngles(JNIEnv* env, jclass /*clazz*/) {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    std::time_t t = static_cast<std::time_t>(ms / 1000);
    struct tm buf;
#ifdef _WIN32
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

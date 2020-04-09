#include <jni.h>
#include <string>
#include "androidlog.h"

extern "C" {//指明当前C++代码调用其他C
#include <libavutil/avutil.h>
}

JNIEnv *env = nullptr;
static const char* path = "com/jamestony/ffmpeg_diary/player/StephenPlayer";

extern "C" JNIEXPORT jstring JNICALL
Java_com_jamestony_ffmpeg_1diary_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject obj) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}


JNINativeMethod method[] = {{},{}};


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    int ret = vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (ret != 0) {
        LOGE("getenv", "error");
    }
    return 1;
}







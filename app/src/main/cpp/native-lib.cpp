#include <jni.h>
#include <string>

extern "C" {//指明当前C++代码调用其他C
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jamestony_ffmpeg_1diary_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject obj) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}

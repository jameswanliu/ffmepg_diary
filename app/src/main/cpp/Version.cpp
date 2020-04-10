//
// Created by kevin on 2020/4/10.
//

#include <jni.h>

extern "C" {//指明当前C++代码调用其他C
#include <libavutil/avutil.h>
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_jamestony_ffmpeg_1diary_version_Version_getVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(av_version_info());
}

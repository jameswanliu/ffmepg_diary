//
// Created by kevin on 2020/4/18.
//

#ifndef FFMPEG_DIARY_JAVACALLHELPER_H
#define FFMPEG_DIARY_JAVACALLHELPER_H

#include <jni.h>

class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *vm, JNIEnv *env, jobject obj);

    ~JavaCallHelper();

    void callbackError(int threadMode,int code);

    void callbackProgress(int threadMode,int progress);

    void callbackPrepare(int threadMode);

    JavaVM *vm;
    JNIEnv *env;
    jobject jobj;
    jmethodID jonErrorId;
    jmethodID jonPrepareId;
    jmethodID jonProgressId;
};


#endif //FFMPEG_DIARY_JAVACALLHELPER_H

//
// Created by kevin on 2020/4/18.
//

#include <include/Const.h>
#include "JavaCallHelper.h"


JavaCallHelper::JavaCallHelper(JavaVM *vm, JNIEnv *env, jobject obj) : vm(vm), env(env) {
    jobj = env->NewGlobalRef(obj);
    jclass jcls = env->GetObjectClass(jobj);
    jonErrorId = env->GetMethodID(jcls, "error", "(I)V");
    jonPrepareId = env->GetMethodID(jcls, "error", "()V");
    jonProgressId = env->GetMethodID(jcls, "error", "(I)V");
}


void JavaCallHelper::callbackError(int threadMode, int code) {
    if (threadMode == THREAD_CHILD) {
        JNIEnv *JNIEnv;
        if (vm->AttachCurrentThread(&JNIEnv, 0) != JNI_OK) {
            return;
        }
        JNIEnv->CallVoidMethod(jobj, jonErrorId, code);
        vm->DetachCurrentThread();
    } else {
        env->CallVoidMethod(jobj, jonErrorId, code);
    }

}


void JavaCallHelper::callbackProgress(int threadMode, int progress) {
    if (threadMode == THREAD_CHILD) {
        JNIEnv *JNIEnv;
        if (vm->AttachCurrentThread(&JNIEnv, 0) != JNI_OK) {
            return;
        }
        JNIEnv->CallVoidMethod(jobj, jonProgressId, progress);
        vm->DetachCurrentThread();
    } else {
        env->CallVoidMethod(jobj, jonProgressId, progress);
    }

}


void JavaCallHelper::callbackPrepare(int threadMode) {
    if (threadMode == THREAD_CHILD) {
        JNIEnv *JNIEnv;
        if (vm->AttachCurrentThread(&JNIEnv, 0) != JNI_OK) {
            return;
        }
        JNIEnv->CallVoidMethod(jobj, jonPrepareId);
        vm->DetachCurrentThread();
    } else {
        env->CallVoidMethod(jobj, jonPrepareId);
    }

}


JavaCallHelper::~JavaCallHelper() {

}



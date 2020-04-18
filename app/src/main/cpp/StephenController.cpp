//
// Created by kevin on 2020/4/14.
//

#include <include/androidlog.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <include/Const.h>
#include "include/StephenController.h"

StephenController::StephenController() {
}

void StephenController::startPlay() {

}


void *prepare(void *arg) {
    StephenController *stephenController = static_cast<StephenController *>(arg);
    stephenController->prepareFFmpeg();
    return 0;
}


void StephenController::initalFFmpeg(JavaVM *vm, JNIEnv *env, jobject obj,jstring path) {
    url = env->GetStringUTFChars(path,NULL);
    javaCallHelper = new JavaCallHelper(vm, env, obj);
    pthread_create(&pid_create, NULL, prepare, this);
    env->ReleaseStringUTFChars(path,url);
}


int StephenController::prepareFFmpeg() {
    AVFormatContext *avFormatContext;
    int ret = 1;
    avFormatContext = avformat_alloc_context();
    AVCodecContext *avCodecContext = nullptr;
    AVDictionary *avDictionary = nullptr;
    av_dict_set(&avDictionary, "timeout", "3000000", 0);
    ret = avformat_open_input(&avFormatContext, url, NULL, &avDictionary);
    if (ret != 0) {
        javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_FILE);
        return ret;
    }
    ret = avformat_find_stream_info(avFormatContext, NULL);
    if (ret < 0) {
        javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAM);
        return ret;
    }
    AVCodec *avCodec = nullptr;
    AVCodecParameters *parameters = nullptr;
    int index = -1;
    for (int i = 0; avFormatContext->nb_streams; ++i) {//在最新的流数据中找到视频流
        int type = avFormatContext->streams[i]->codecpar->codec_type;
        parameters = avFormatContext->streams[index]->codecpar;//实例化解码器参数
        if (!parameters) {
            javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_DECODE_PARAMS_CONTEXT_FAIL);
            return ret;
        }
        avCodec = avcodec_find_decoder(parameters->codec_id);//查找视频解码器
        if (!avCodec) {
            javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return ret;
        }

    }
    avCodecContext = avcodec_alloc_context3(avCodec);
    if (!avCodecContext) {
        javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_ALLOC_DECODE_CONTEXT_FAIL);
        return ret;
    }
    ret = avcodec_open2(avCodecContext, avCodec, NULL);//打开视频编码器
    if (ret != JNI_OK) {
        javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
        return ret;
    }

    if (parameters->codec_type == AVMEDIA_TYPE_AUDIO) {
        audioChanel = new AudioChanel(parameters->codec_id, javaCallHelper, avCodecContext);
    }
    if (parameters->codec_type == AVMEDIA_TYPE_VIDEO) {
        videoChanel = new VideoChanel(parameters->codec_id, javaCallHelper, avCodecContext);
    }

    //回调初始化成功
    javaCallHelper->callbackPrepare(THREAD_CHILD);
    return ret;
}


StephenController::~StephenController() {
    delete (url);
    if (&mutex != NULL) pthread_mutex_destroy(&mutex);
    if (&condt != NULL) pthread_cond_destroy(&condt);
}

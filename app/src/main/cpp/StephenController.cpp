//
// Created by kevin on 2020/4/14.
//

#include <include/androidlog.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <include/Const.h>

extern "C" {
#include <include/libavutil/time.h>
}

#include "include/StephenController.h"

AVFormatContext *avFormatContext;

StephenController::StephenController() {
}


void *prepare(void *arg) {
    StephenController *stephenController = static_cast<StephenController *>(arg);
    stephenController->prepareFFmpeg();
    return 0;
}


void StephenController::initalFFmpeg(JavaVM *vm, JNIEnv *env, jobject obj, jstring path) {
    url = env->GetStringUTFChars(path, NULL);
    javaCallHelper = new JavaCallHelper(vm, env, obj);
    pthread_create(&pid_create, NULL, prepare, this);
    env->ReleaseStringUTFChars(path, url);
}


int StephenController::prepareFFmpeg() {
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
    int vedioIndex = -1;
    int audioIndex = -1;
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

        if (type == AVMEDIA_TYPE_VIDEO) {
            vedioIndex = i;
        } else if (type == AVMEDIA_TYPE_AUDIO) {
            audioIndex = i;
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
        audioChanel = new AudioChanel(audioIndex, javaCallHelper, avCodecContext);
    }
    if (parameters->codec_type == AVMEDIA_TYPE_VIDEO) {
        videoChanel = new VideoChanel(vedioIndex, javaCallHelper, avCodecContext);
        videoChanel->setRenderFrame(renderFrame);
    }

    //回调初始化成功
    javaCallHelper->callbackPrepare(THREAD_CHILD);
    return ret;
}


void *dispatchThread(void *args) {
    StephenController *stephenController = static_cast<StephenController *>(args);
    stephenController->dispatchPacket();
    return 0;
}


/**
 * 生产packet 放入队列
 */
void StephenController::dispatchPacket() {
    int ret = 0;
    while (isPlaying) {
        if (audioChanel && audioChanel->avpacketQueue.size() > 100) {
            av_usleep(1000 * 10);//生产快于渲染 所以延迟10毫秒
            continue;
        }

        if (videoChanel && videoChanel->avpacketQueue.size() > 100) {
            av_usleep(1000 * 10);
            continue;
        }
        AVPacket *avPacket = av_packet_alloc();
        ret = av_read_frame(avFormatContext, avPacket);

        if (ret == 0) {
            if (audioChanel && avPacket->stream_index == audioChanel->chanelId) {
                audioChanel->avpacketQueue.enQueue(avPacket);
            } else if (videoChanel && avPacket->stream_index == videoChanel->chanelId) {
                videoChanel->avpacketQueue.enQueue(avPacket);
            }
        } else if (ret == AVERROR_EOF) {
            if (audioChanel->avpacketQueue.empty() && audioChanel->avFrameQueue.empty()
                && videoChanel->avpacketQueue.empty() && videoChanel->avFrameQueue.empty()) {
                LOGI("controller", "解压完毕");
                break;
            }
        } else {
            break;
        }
    }

    isPlaying = 0;
    audioChanel->stop();
    videoChanel->stop();
}


void StephenController::setRenderFrame(RenderFrame renderFrame1) {
    this->renderFrame = renderFrame1;
}

/**
 * 开辟解码线程
 */
void StephenController::start() {
    isPlaying = 1;
    pthread_create(&pid_dispatch_packet, NULL, dispatchThread, this);
}



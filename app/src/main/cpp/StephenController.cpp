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

StephenController::StephenController(JavaCallHelper *javaCallHelper) {
    this->javaCallHelper = javaCallHelper;
}


void *prepare(void *arg) {
    StephenController *stephenController = static_cast<StephenController *>(arg);
    stephenController->prepareFFmpeg();
    return 0;
}


void StephenController::initalFFmpeg(JNIEnv *env, jstring path) {
    const char *path_ = env->GetStringUTFChars(path, NULL);
    url = new char[strlen(path_) + 1];
    strcpy(const_cast<char *>(url), path_);
    pthread_create(&pid_create, NULL, prepare, this);
//    int ret = prepareFFmpeg();
//    LOGI("ret", "prepareFFmpeg = %d" ,ret);
    env->ReleaseStringUTFChars(path, path_);
}


int StephenController::prepareFFmpeg() {
    int ret = -1;
    avformat_network_init();
    avFormatContext = avformat_alloc_context();
    AVDictionary *opts = NULL;
    ret = av_dict_set(&opts, "timeout", "3000000", 0);
    if (ret < 0) {
        javaCallHelper->callbackError(THREAD_CHILD, FFMEPG_SET_AVDICTIONARY_FAIL);
        return ret;
    }
    ret = avformat_open_input(&avFormatContext, url, NULL, &opts);
    if (ret != 0) {
        javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_FILE);
        return ret;
    }
    ret = avformat_find_stream_info(avFormatContext, NULL);
    if (ret < 0) {
        javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAM);
        return ret;
    }
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {//在最新的流数据中找到视频流
        AVCodecParameters *parameters = avFormatContext->streams[i]->codecpar;//实例化解码器参数
        if (!parameters) {
            javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_DECODE_PARAMS_CONTEXT_FAIL);
            return ret;
        }
        AVCodec *avCodec = avcodec_find_decoder(parameters->codec_id);//查找视频解码器
        if (!avCodec) {
            javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return ret;
        }

        AVCodecContext *video_codec_context = avcodec_alloc_context3(avCodec);
        if (!video_codec_context) {
            javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_ALLOC_DECODE_CONTEXT_FAIL);
            return ret;
        }
        ret = avcodec_parameters_to_context(video_codec_context,
                                            parameters);//将解码参数设置到AVCodecContext 中
        if (ret < 0) {
            javaCallHelper->callbackError(THREAD_CHILD, FFMEPG_AVCODEC_PARAMETERS_TO_CONTEXT);
            return ret;
        }

        ret = avcodec_open2(video_codec_context, avCodec, NULL);//打开视频编码器
        if (ret != JNI_OK) {
            javaCallHelper->callbackError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return ret;
        }


        if (parameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGI("audioChanel=", "index = %d", i);
            audioChanel = new AudioChanel(i, javaCallHelper, video_codec_context);
        }
        if (parameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGI("videoChanel=", "index = %d", i);
            videoChanel = new VideoChanel(i, javaCallHelper, video_codec_context);
            videoChanel->setRenderFrame(renderFrame);
        }
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
                LOGI("audio stream_index", "index = %d", avPacket->stream_index);
                audioChanel->avpacketQueue.enQueue(avPacket);
            } else if (videoChanel && avPacket->stream_index == videoChanel->chanelId) {
                LOGI("video stream_index", "index = %d", avPacket->stream_index);
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
    if (audioChanel) {
        audioChanel->play();
    }
    if (videoChanel) {
        videoChanel->play();
    }
    pthread_create(&pid_dispatch_packet, NULL, dispatchThread, this);
}



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


/**
 * controller
 * 作用：
 *  1.开辟子线程初始化ffmpeg(
 *     （1）初始化AvformatContext
 *      (2) 设置打开文件参数AvDictionary
 *      (3) 打开文件
 *      (4) 找到文件中的音视频流信息
 *      (5) 遍历avformatCotext 中的流 找到视频流 音频流
 *      (6) 根据每个流找到 视频解码器参数AVCodecParameters
 *      (7) 根据解码器参数 实例化解码器AvCodec
 *      (8) 用解码器Avcodec 实例化解码器上下文AvCodecContext
 *      (9) 将解码器参数AVCodecParameters与AvCodecContext 绑定
 *      (10) 打开解码器avcodec_open)
 *  2.开辟子线程将得到的avpacket 放入各自的音频队列和视频队列中
 * @param javaCallHelper
 */
StephenController::StephenController(JavaCallHelper *javaCallHelper) {
    this->javaCallHelper = javaCallHelper;
}


void *prepare(void *arg) {
    StephenController *stephenController = static_cast<StephenController *>(arg);
    stephenController->prepareFFmpeg();
    return 0;
}


void StephenController::seek(int position) {
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
        AVStream *stream = avFormatContext->streams[i];
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
            audioChanel = new AudioChanel(i, javaCallHelper, video_codec_context,
                                          stream->time_base);//流中获取帧时间基
        }
        if (parameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVRational avgFrameRate = stream->avg_frame_rate;//获取平均帧率
            double fps = av_q2d(avgFrameRate);//将平均帧率 转成double
            LOGI("videoChanel=", "index = %d", i);
            videoChanel = new VideoChanel(i, javaCallHelper, video_codec_context,
                                          stream->time_base);//流中获取帧时间基
            videoChanel->setRenderFrame(renderFrame);
            videoChanel->setFps(fps);
        }
    }
    if (audioChanel) {
        videoChanel->setAudioChanel(audioChanel);
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



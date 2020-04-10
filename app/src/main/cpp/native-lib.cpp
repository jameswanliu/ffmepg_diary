#include <jni.h>
#include <string>
#include "androidlog.h"
#include <android/native_window_jni.h>

extern "C" {//指明当前C++代码调用其他C
#include <libavutil/avutil.h>
#include <include/libavformat/avformat.h>
#include <include/libavutil/imgutils.h>
}

JNIEnv *env = nullptr;
static const char *classPath = "com/jamestony/ffmpeg_diary/player/StephenPlayer";


extern "C" JNIEXPORT jint JNICALL play(JNIEnv *env, jobject obj, jstring path, jobject surface) {
    const char *path_ = env->GetStringUTFChars(path, NULL);//视频路径

    AVFormatContext *avFormatContext;
    ANativeWindow *nativeWindow;
    nativeWindow = ANativeWindow_fromSurface(env, surface);//调用android 的nativewindow
    int ret = -1;
    avFormatContext = avformat_alloc_context();
    AVCodecContext *avCodecContext = nullptr;
    AVDictionary *avDictionary = nullptr;
    av_dict_set(&avDictionary, "timeout", "3000000", 0);
    ret = avformat_open_input(&avFormatContext, path_, NULL, &avDictionary);
    if (ret == AVERROR) {
        LOGE("OPEN", "open video fail");
        return ret;
    }

    ret = avformat_find_stream_info(avFormatContext, &avDictionary);
    if (ret == AVERROR) {
        LOGE("FIND", "find stream error");
        return ret;
    }
    AVCodec *avCodec = nullptr;
    avCodecContext = avcodec_alloc_context3(avCodec);
    int index = -1;
    for (int i = 0; avFormatContext->nb_streams; ++i) {//在最新的流数据中找到视频流
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            index = i;
            break;
        }
    }
    AVCodecParameters *parameters = avFormatContext->streams[index]->codecpar;
    avCodec = avcodec_find_decoder(parameters->codec_id);//查找视频解码器
    avcodec_parameters_to_context(avCodecContext, parameters);
    AVPacket *avPacket = av_packet_alloc();
    ret = avcodec_open2(avCodecContext, avCodec, NULL);//打开视频编码器
    if (ret != JNI_OK) {
        LOGE("AVCODE OPEN", "avcode open error");
        return ret;
    }


    while (av_read_frame(avFormatContext, avPacket)) {
        avcodec_send_packet(avCodecContext, avPacket);
        AVFrame *avFrame = av_frame_alloc();
        int res = avcodec_receive_frame(avCodecContext, avFrame);
        if (res == AVERROR(EAGAIN)) {
            continue;
        } else if (res < 0) {
            LOGE("RECEIVE", "receive frame error");
            break;
        }


        uint8_t *pointers[0];
        int linesizes[0];
        av_image_alloc(pointers, linesizes, avCodecContext->width, avCodecContext->height,
                       avCodecContext->pix_fmt, 1);//分配大小为w和h、像素格式为pix_fmt的图像，
        // 以及相应地填充指针和线条大小。必须使用 av_freep(&pointers[0]) 释放内存



    }


    env->ReleaseStringUTFChars(path, path_);
}

JNINativeMethod method[] = {{"play", "(Ljava/lang/String;Landroid/view/Surface;)I", (void *) play}};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    int ret = vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (ret != JNI_OK) {
        LOGE("getenv", "error");
    }
    ret = env->RegisterNatives(env->FindClass(classPath), method,
                               sizeof(method) / sizeof(JNINativeMethod));
    if (ret != JNI_OK) {
        LOGE("regist", "regist_error");
    }
    if (ret != JNI_OK) {
        LOGE("regist", "registSplashPath_error");
    }
    avformat_network_init();
    return 1;
}







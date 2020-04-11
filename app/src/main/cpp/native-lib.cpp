#include <jni.h>
#include <string>
#include "androidlog.h"
#include <android/native_window_jni.h>
#include <include/libswscale/swscale.h>

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
    if (ret != 0) {
        LOGE("OPEN", "open video fail");
        return ret;
    }

    ret = avformat_find_stream_info(avFormatContext, &avDictionary);
    if (ret < 0) {
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
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt,
                                            avCodecContext->width, avCodecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
    ANativeWindow_setBuffersGeometry(nativeWindow, avCodecContext->width, avCodecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer outBuffer;

    while (av_read_frame(avFormatContext, avPacket) >= 0) {
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

        //分配大小为w和h、像素格式为pix_fmt的图像，
        // 以及相应地填充指针和线条大小。必须使用 av_freep(&pointers[0]) 释放内存
        av_image_alloc(pointers, linesizes, avCodecContext->width, avCodecContext->height,
                       AV_PIX_FMT_RGBA, 1);


        // 通过swscontext 将yuv 数据转换成rgb 数据 注意 avframe 通过receive_frame已经获取到了数据 ，
        // 输入源是frame->data,pointer 是通过av_image_alloc
        //申请的格式为AV_PIX_FMT_RGBA 格式的图像，linesizes是申请的行数，所以 pointer 是 输出源
        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *>(avFrame->data),
                  avFrame->linesize, 0, avFrame->height,  pointers,
                 linesizes);

        //与 获取图像的bitmap一样 首先要上锁 然后获取 outbuffer
        //将上一步的输出源pointer 拷贝到 nativewindow 中的buffer 中
        //为了对齐需要将 输出源一行一行地拷贝到 nativewindow 中
        ANativeWindow_lock(nativeWindow, &outBuffer, NULL);


        //一行字节数
        int destStride = outBuffer.stride * 4;
        //开始地址 需要被赋值的指针
        uint8_t *start = static_cast<uint8_t *>(outBuffer.bits);
        //被拷贝的行数
        int srcStride = linesizes[0];
        //被拷贝的指针
        uint8_t *src_data = pointers[0];
        for (int i = 0; i < outBuffer.height; i++) {

            //将srcdata 数据拷贝到 start
            memcpy(start + i * destStride, src_data + i * srcStride, destStride);
        }
        ANativeWindow_unlockAndPost(nativeWindow);
        av_frame_free(&avFrame);
    }
    ANativeWindow_release(nativeWindow);
    avcodec_close(avCodecContext);
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







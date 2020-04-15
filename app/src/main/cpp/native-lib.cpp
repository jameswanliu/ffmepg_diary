#include <jni.h>
#include <string>
#include "androidlog.h"
#include <android/native_window_jni.h>
#include <unistd.h>
#include <__std_stream>


extern "C" {//指明当前C++代码调用其他C
#include <libavutil/avutil.h>
#include <include/libavformat/avformat.h>
#include <include/libavutil/imgutils.h>
#include <include/libswresample/swresample.h>
#include <include/libswscale/swscale.h>
}

JNIEnv *env = nullptr;
static const char *classPath = "com/jamestony/ffmpeg_diary/player/StephenPlayer";

static const char *classAudioPath = "com/jamestony/ffmpeg_diary/player/SteAudioPlayer";

using namespace std;

extern "C" JNIEXPORT jint playAudio(JNIEnv *env, jobject obj, jstring path, jstring output) {
    int ret = -1;
    const char *path_ = env->GetStringUTFChars(path, NULL);
    const char *output_ = env->GetStringUTFChars(output, NULL);
    AVFormatContext *avFormatContext = avformat_alloc_context();

    AVDictionary *avDictionary = nullptr;
    av_dict_set(&avDictionary, "timeout", "3000000", 0);
    ret = avformat_open_input(&avFormatContext, path_, NULL, &avDictionary);
    if (ret != 0) {
        LOGE("OPEN", "open audio fail");
        return ret;
    }

    ret = avformat_find_stream_info(avFormatContext, NULL);
    if (ret < 0) {
        LOGE("OPEN", "find stream_info fail");
        return ret;
    }

    AVCodecContext *avCodecContext = nullptr;

    int stream_audio_index = -1;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_audio_index = i;
            break;
        }
    }
    AVCodecParameters *parameters = avFormatContext->streams[stream_audio_index]->codecpar;
    AVCodec *avCodec = avcodec_find_decoder(parameters->codec_id);
    avCodecContext = avcodec_alloc_context3(avCodec);
    avcodec_parameters_to_context(avCodecContext, parameters);
    avcodec_open2(avCodecContext, avCodec, NULL);
    AVPacket *avPacket = av_packet_alloc();

    SwrContext *swrContext = swr_alloc();


    int in_sampleRate = avCodecContext->sample_rate;
    int64_t in_ch_layout = avCodecContext->channel_layout;
    AVSampleFormat in_sample_fmt = avCodecContext->sample_fmt;

    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;//左声道和右声道
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 48000;
    /**
     * * @param s               existing Swr context if available, or NULL if not
     * @param out_ch_layout   output channel layout (AV_CH_LAYOUT_*)
     * @param out_sample_fmt  output sample format (AV_SAMPLE_FMT_*).
     * @param out_sample_rate output sample rate (frequency in Hz)
     * @param in_ch_layout    input channel layout (AV_CH_LAYOUT_*)
     * @param in_sample_fmt   input sample format (AV_SAMPLE_FMT_*).
     * @param in_sample_rate  input sample rate (frequency in Hz)
     * @param log_offset      logging level offset
     * @param log_ctx         parent logging context, can be NULL
     */
    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample_fmt, out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sampleRate, 0, NULL);


    //输出的文件指针
    FILE *file = fopen(output_, "wb");

    //每一帧写入文件的大小
    uint8_t *outbufferb = (uint8_t *) av_malloc(2 * 48000);
    //初始化转换上下文
    swr_init(swrContext);

    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        int ret;
        avcodec_send_packet(avCodecContext, avPacket);
        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            LOGI("OVER", "解码完成")
            break;
        }

        if (stream_audio_index != avPacket->stream_index) {
            continue;
        }
        LOGI("OVER", "解码中")

        //将封装数据 解析成pcm数据
        swr_convert(swrContext, &outbufferb, 2 * 48000, (const uint8_t **) avFrame->data,
                    avFrame->nb_samples);


        //获取当前通道数
        int nb_channels = av_get_channel_layout_nb_channels(out_ch_layout);

        //获取缓冲区大小
        int out_buffer_size = av_samples_get_buffer_size(NULL, nb_channels,
                                                                     avFrame->nb_samples,
                                                                     out_sample_fmt, 1);

        //将outbufferb 写进文件 1是字节数
        fwrite(outbufferb, 1, out_buffer_size, file);
        av_frame_free(&avFrame);
    }

    free(file);
    swr_free(&swrContext);
    av_free(outbufferb);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);
    av_packet_free(&avPacket);
    env->ReleaseStringUTFChars(output, output_);
    env->ReleaseStringUTFChars(path, path_);
    return ret;

}



extern "C" JNIEXPORT jint playVideo(JNIEnv *env, jobject obj, jstring path, jobject surface) {
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

    ret = avformat_find_stream_info(avFormatContext, NULL);
    if (ret < 0) {
        LOGE("FIND", "find stream error");
        return ret;
    }
    AVCodec *avCodec = nullptr;
    int index = -1;
    for (int i = 0; avFormatContext->nb_streams; ++i) {//在最新的流数据中找到视频流
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            index = i;
            break;
        }
    }
    AVCodecParameters *parameters = avFormatContext->streams[index]->codecpar;//实例化解码器参数
    avCodec = avcodec_find_decoder(parameters->codec_id);//查找视频解码器
    avCodecContext = avcodec_alloc_context3(avCodec);
    avcodec_parameters_to_context(avCodecContext, parameters);//将解码参数设置到解码器上下文中
    AVPacket *avPacket = av_packet_alloc();
    ret = avcodec_open2(avCodecContext, avCodec, NULL);//打开视频编码器
    if (ret != JNI_OK) {
        LOGE("AVCODE OPEN", "avcode open error");
        return ret;
    }


    //实例化转换上下文
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt,
                                            avCodecContext->width, avCodecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
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
        sws_scale(swsContext, (const uint8_t *const *) (avFrame->data),
                  avFrame->linesize, 0, avFrame->height, pointers,
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
//        usleep(1000 * 16);
        usleep((unsigned long) (1000 * 40 * 1));
        av_frame_free(&avFrame);
    }
    ANativeWindow_release(nativeWindow);
    avcodec_close(avCodecContext);
    av_packet_free(&avPacket);
    avformat_close_input(&avFormatContext);
    sws_freeContext(swsContext);
//    delete (avPacketQueue);
    env->ReleaseStringUTFChars(path, path_);
}

JNINativeMethod methods[] = {{"playAudio", "(Ljava/lang/String;Ljava/lang/String;)I", (void *) playAudio}};
JNINativeMethod method[] = {{"playVideo", "(Ljava/lang/String;Landroid/view/Surface;)I", (void *) playVideo}};

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

    ret = env->RegisterNatives(env->FindClass(classAudioPath), methods,
                               sizeof(methods) / sizeof(JNINativeMethod));

    if (ret != JNI_OK) {
        LOGE("regist", "registclassAudioPath _error");
    }
    avformat_network_init();
    return JNI_VERSION_1_6;
}







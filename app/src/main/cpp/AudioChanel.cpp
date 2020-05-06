//
// Created by kevin on 2020/4/16.
//

#include <include/androidlog.h>
#include "AudioChanel.h"
#include "BaseChanel.h"

extern "C" {
#include <include/libswresample/swresample.h>
#include "include/libavcodec/avcodec.h"
#include <include/libavutil/time.h>
}


AudioChanel::AudioChanel(int chanelId, JavaCallHelper *javaCallHelper,
                         AVCodecContext *avCodecContext, AVRational time_base) : BaseChanel(
        chanelId, javaCallHelper,
        avCodecContext, time_base) {

    //每个样本的字节数
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_chanel = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_rate = 44100;
    //每一帧的buffer = 采样频率44100 * 采样数16位（2个字节）*通道数（2）
    buffer = (uint8_t *) malloc(static_cast<size_t>(out_sample_rate * out_samplesize * out_chanel));

}


AudioChanel::~AudioChanel() {

}

void *decodeAudioThread(void *args) {
    AudioChanel *audioChanel = static_cast<AudioChanel *>(args);
    audioChanel->decodePacket();
    return 0;
}

void AudioChanel::decodePacket() {
    AVPacket *avPacket;
    int ret = -1;
    while (isPlay) {
        LOGI("decode audio packet", "isplay");
        if (!isPlay) {
            break;
        }
        ret = avpacketQueue.deQueue(avPacket);
        if (!ret) {
            continue;
        }

        LOGI("decode deQueue ret =", "%d=", ret);
        ret = avcodec_send_packet(avCodecContext, avPacket);
        freeAvPacket(avPacket);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        }
        if (ret < 0) {
            break;
        }


        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        LOGI("decode receive ret =", "%d", ret);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        }
        if (ret < 0) {
            break;
        }
        avFrameQueue.enQueue(avFrame);
        while (avFrameQueue.size() > 100 && isPlay) {
            LOGI("avframequeue =", "full");
            av_usleep(1000 * 10);
            continue;
        }
    }
    freeAvPacket(avPacket);
}


int AudioChanel::getPcm() {
    AVFrame *avFrame = NULL;
    int dataSize = 0;
    int ret;
    while (isPlay) {
        ret = avFrameQueue.deQueue(avFrame);
        LOGI("getpcm ret =", "%d", ret);
        if (!isPlay) {
            break;
        }
        if (!ret) {
            continue;
        }

        /**
         * 重新缩放 得到输出数量
         */
        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swrContext, avFrame->sample_rate) + avFrame->nb_samples,
                out_sample_rate,
                avFrame->sample_rate,
                AV_ROUND_UP);


        LOGI("desnbSample =", "%d", dst_nb_samples);

        /**
         * 每个通道输出的样本数
         */
        int nb = swr_convert(swrContext, &buffer, dst_nb_samples, (const uint8_t **) avFrame->data,
                             avFrame->nb_samples);
        LOGI("nb =", "%d", nb);
        /**
         * 每个通道的样本数 * 通道数 * 每个样本的字节数
         */
        dataSize = nb * out_chanel * out_samplesize;
        LOGI("dataSize =", "%d", dataSize);


        /**
         * 获取实时的显示时间（显示时间戳*时间单位）
         */
        LOGI("a address","%p",&time_base);
        clock = avFrame->pts * av_q2d(time_base);

        /**
         * 获取到dataSize 和 Buffer地址 后一定要break
         */
        break;
    }
    freeAvFrame(avFrame);
    return dataSize;
}


void *playThread(void *args) {
    AudioChanel *audioChanel = static_cast<AudioChanel *>(args);
    audioChanel->initOpensles();
    return 0;
}


void AudioChanel::play() {
    swrContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                    out_sample_rate, avCodecContext->channel_layout,
                                    avCodecContext->sample_fmt,
                                    avCodecContext->sample_rate, 0, 0);
    swr_init(swrContext);
    avFrameQueue.setWork(1);
    avpacketQueue.setWork(1);
    isPlay = true;
    pthread_create(&pthread_audio_play, NULL, playThread, this);
    pthread_create(&pthread_audio_decode, NULL, decodeAudioThread, this);
}


void AudioChanel::stop() {

}


void bqPlayerCallback(SLAndroidSimpleBufferQueueItf qb, void *context) {
    AudioChanel *audioChanel = static_cast<AudioChanel *>(context);
    int size = audioChanel->getPcm();
    (*qb)->Enqueue(qb, audioChanel->buffer, (SLuint32) size);
}


/**
 *
 */
void AudioChanel::initOpensles() {

    //sl播放引擎
    SLEngineItf slEngineItf;

    //引擎对象
    SLObjectItf slEnginObject;

    //混音器对象
    SLObjectItf outputMixObject;


    //播放器对象
    SLObjectItf slAudioPlayer;


    //播放器回调接口
    SLPlayItf slplayInterface;


    //缓冲队列接口
    SLAndroidSimpleBufferQueueItf slAndroidSimpleBufferQueueItf;

    SLresult sLresult;

    sLresult = slCreateEngine(&slEnginObject, 0, NULL, 0, NULL, NULL);

    LOGI("create engine=", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
    sLresult = (*slEnginObject)->Realize(slEnginObject, SL_BOOLEAN_FALSE);
    LOGI("Realize engineobject=", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
    sLresult = (*slEnginObject)->GetInterface(slEnginObject, SL_IID_ENGINE, &slEngineItf);
    LOGI("get engineobject inter=", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }

    sLresult = (*slEngineItf)->CreateOutputMix(slEngineItf, &outputMixObject, 0, 0, 0);
    LOGI("create output =", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
    sLresult = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    LOGI("realize output =", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    //pcm数据格式
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM//播放pcm格式的数据
            , 2,//2个声道（立体声）
                            SL_SAMPLINGRATE_44_1, //44100hz的频率
                            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
                            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
                            SL_BYTEORDER_LITTLEENDIAN//小端模式
    };
    //
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    SLDataSink audioSnk = {&outputMix, NULL};
    SLDataSource slDataSource = {&android_queue, &pcm};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    sLresult = (*slEngineItf)->CreateAudioPlayer(slEngineItf, &slAudioPlayer// //播放器
            , &slDataSource//播放器参数  播放缓冲队列   播放格式
            , &audioSnk,//播放缓冲区
                                                 1,//播放接口回调个数
                                                 ids,//设置播放队列ID
                                                 req//是否采用内置的播放队列
    );
    LOGI("create player =", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
    //初始化播放器
    sLresult = (*slAudioPlayer)->Realize(slAudioPlayer, SL_BOOLEAN_FALSE);

    LOGI("realize player =", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
//bqPlayerObject   这个对象
//    得到接口后调用  获取Player接口
    sLresult = (*slAudioPlayer)->GetInterface(slAudioPlayer, SL_IID_PLAY, &slplayInterface);
    LOGI("get player inter=", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
//    获得播放器接口
    sLresult = (*slAudioPlayer)->GetInterface(slAudioPlayer, SL_IID_BUFFERQUEUE,
                                              &slAndroidSimpleBufferQueueItf);
    LOGI("get player bfinter=", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }


    sLresult = (*slAndroidSimpleBufferQueueItf)->RegisterCallback(slAndroidSimpleBufferQueueItf,
                                                                  bqPlayerCallback, this);
    LOGI("regist bf callback=", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }
    //    设置播放状态
    sLresult = (*slplayInterface)->SetPlayState(slplayInterface, SL_PLAYSTATE_PLAYING);
    LOGI("set play state", "%d", sLresult);
    if (SL_RESULT_SUCCESS != sLresult) {
        return;
    }

    bqPlayerCallback(slAndroidSimpleBufferQueueItf, this);
}
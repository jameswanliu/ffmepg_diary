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
                         AVCodecContext *avCodecContext) : BaseChanel(chanelId, javaCallHelper,
                                                                      avCodecContext) {

    //每个样本的字节数
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_chanel = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_rate = 44100;
    //每一帧的buffer = 采样频率44100 * 采样数16位（2个字节）*通道数（2）
    buffer = (uint8_t *) malloc(out_sample_rate * out_samplesize * out_chanel);

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
        LOGI("decode audio packet","isplay");
        if (!isPlay) {
            break;
        }
        ret = avpacketQueue.deQueue(avPacket);
        if (!ret) {
            continue;
        }

        LOGI("deQueue ret =","%d=",ret);
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
        LOGI("receive ret =","%d",ret);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        }
        if (ret < 0) {
            break;
        }
        while (avpacketQueue.size() > 100 && isPlay) {
            LOGI("full","%d=",isPlay);
            av_usleep(1000 * 10);
            continue;
        }
        avFrameQueue.enQueue(avFrame);
    }
    freeAvPacket(avPacket);
}


int AudioChanel::getPcm() {
    AVFrame *avFrame;
    int dataSize = 0;
    int ret;
    while (isPlay) {
        if (!isPlay) {
            break;
        }
        ret = avFrameQueue.deQueue(avFrame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        }
        if (!ret) {
            continue;
        }

        /**
         * 重新缩放 得到输出数量
         */
        uint64_t desnbSample = (uint64_t) av_rescale_rnd(
                swr_get_delay(swrContext, avFrame->sample_rate) + avFrame->nb_samples,
                out_sample_rate, avFrame->sample_rate, AV_ROUND_UP);


        /**
         * 每个通道输出的样本数
         */
        int nb = swr_convert(swrContext, &buffer, desnbSample, (const uint8_t **) avFrame->data,
                             avFrame->nb_samples);

        if (nb < 0) {
            continue;
        }

        /**
         * 每个通道的样本数 * 通道数 * 每个样本的字节数
         */
        dataSize = nb * out_chanel * out_samplesize;
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
    isPlay = true;
    swr_init(swrContext);
    avFrameQueue.setWork(1);
    avpacketQueue.setWork(1);
    pthread_create(&pthread_audio_play, NULL, playThread, this);
    pthread_create(&pthread_audio_decode, NULL, decodeAudioThread, this);
}


void AudioChanel::stop() {

}


void bqPlayerCallback(SLAndroidSimpleBufferQueueItf qb, void *context) {
    AudioChanel *audioChanel = static_cast<AudioChanel *>(context);
    int size = audioChanel->getPcm();
    LOGE("info","%d",size);
    if (size > 0) {
        LOGE("info size","%d",size);
        (*qb)->Enqueue(qb, audioChanel->buffer, size);
    }
}


int AudioChanel::initOpensles() {

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

    if (sLresult != SL_RESULT_SUCCESS) {
        return sLresult;
    }
    sLresult = (*slEnginObject)->Realize(slEnginObject, SL_BOOLEAN_FALSE);
    if (sLresult != SL_RESULT_SUCCESS) {
        return sLresult;
    }

    sLresult = (*slEnginObject)->GetInterface(slEnginObject, SL_IID_ENGINE, &slEngineItf);
    if (sLresult != SL_RESULT_SUCCESS) {
        return sLresult;
    }

    sLresult = (*slEngineItf)->CreateOutputMix(slEngineItf, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != sLresult) {
        return sLresult;
    }
    sLresult = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != sLresult) {
        return sLresult;
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
    (*slEngineItf)->CreateAudioPlayer(slEngineItf, &slAudioPlayer// //播放器
            , &slDataSource//播放器参数  播放缓冲队列   播放格式
            , &audioSnk,//播放缓冲区
                                      1,//播放接口回调个数
                                      ids,//设置播放队列ID
                                      req//是否采用内置的播放队列
    );
    //初始化播放器
    (*slAudioPlayer)->Realize(slAudioPlayer, SL_BOOLEAN_FALSE);
//bqPlayerObject   这个对象
//    得到接口后调用  获取Player接口
    (*slAudioPlayer)->GetInterface(slAudioPlayer, SL_IID_PLAY, &slplayInterface);
//    获得播放器接口
    (*slAudioPlayer)->GetInterface(slAudioPlayer, SL_IID_BUFFERQUEUE,
                                   &slAndroidSimpleBufferQueueItf);

    (*slAndroidSimpleBufferQueueItf)->RegisterCallback(slAndroidSimpleBufferQueueItf,
                                                       bqPlayerCallback, this);
    //    设置播放状态
    (*slplayInterface)->SetPlayState(slplayInterface, SL_PLAYSTATE_PLAYING);
    bqPlayerCallback(slAndroidSimpleBufferQueueItf, this);
    return sLresult;
}
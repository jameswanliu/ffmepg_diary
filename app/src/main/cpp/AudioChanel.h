//
// Created by kevin on 2020/4/16.
//

#ifndef FFMPEG_DIARY_AUDIOCHANEL_H
#define FFMPEG_DIARY_AUDIOCHANEL_H

extern "C"{
#include <include/libswresample/swresample.h>
};

#include "SLES/OpenSLES_Android.h"
#include "BaseChanel.h"

class AudioChanel : public BaseChanel {
public:
    AudioChanel(int chanelId, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext);

    ~AudioChanel();

    virtual void play();

    void decodePacket();

    int getPcm();

    virtual void stop();

    int initOpensles();
    uint8_t *buffer;

private:
    pthread_t pthread_audio_decode;
    pthread_t pthread_audio_play;
    int out_chanel;
    int out_samplesize;
    int out_sample_rate;
    SwrContext  *swrContext;
};


#endif //FFMPEG_DIARY_AUDIOCHANEL_H

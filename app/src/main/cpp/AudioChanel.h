//
// Created by kevin on 2020/4/16.
//

#ifndef FFMPEG_DIARY_AUDIOCHANEL_H
#define FFMPEG_DIARY_AUDIOCHANEL_H

#include <include/libswresample/swresample.h>
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

    SLObjectItf *slPlayer;

    int initOpensles();


private:
    pthread_t pthread_audio_decode;
    pthread_t pthread_audio_play;
    int out_chanel;
    int out_samplesize;
    int out_sample_rate;
    SwrContext  *swrContext;
    uint8_t *buffer;
};


#endif //FFMPEG_DIARY_AUDIOCHANEL_H

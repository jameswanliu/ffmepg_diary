//
// Created by kevin on 2020/4/16.
//

#include "AudioChanel.h"
#include ""
extern "C"{
#include "include/libavcodec/avcodec.h"
}



AudioChanel::AudioChanel(int chanelId, JavaCallHelper *javaCallHelper,
                         AVCodecContext *avCodecContext) : BaseChanel(chanelId, javaCallHelper,
                                                                      avCodecContext) {
}


AudioChanel::~AudioChanel() {

}

void *decodeThread(void *args){
    AudioChanel *audioChanel = static_cast<AudioChanel*>(args);
    audioChanel->decodePacket();

}

void AudioChanel::decodePacket() {
    while (isPlay){




    }

}


void AudioChanel::sysncPlay() {

}



void AudioChanel::play() {




}


void AudioChanel::stop() {

}


int AudioChanel::initOpensles() {





}
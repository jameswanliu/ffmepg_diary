//
// Created by kevin on 2020/4/16.
//

#include "AudioChanel.h"


AudioChanel::AudioChanel(int chanelId, JavaCallHelper *javaCallHelper,
                         AVCodecContext *avCodecContext) : BaseChanel(chanelId, javaCallHelper,
                                                                      avCodecContext) {
}


AudioChanel::~AudioChanel() {

}

void AudioChanel::start() {

}


void AudioChanel::stop() {

}
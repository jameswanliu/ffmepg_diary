//
// Created by kevin on 2020/4/18.
//

#include "VideoChanel.h"
#include "JavaCallHelper.h"
#include "include/libavcodec/avcodec.h"
#include "BaseChanel.h"


VideoChanel::VideoChanel(int chanelId, JavaCallHelper *javaCallHelper,
                         AVCodecContext *avCodecContext) : BaseChanel(chanelId, javaCallHelper,
                                                                      avCodecContext) {

}


VideoChanel::~VideoChanel() {

}

void VideoChanel::start() {

}



void VideoChanel::stop() {

}
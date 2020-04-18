//
// Created by kevin on 2020/4/16.
//

#ifndef FFMPEG_DIARY_BASECHANEL_H
#define FFMPEG_DIARY_BASECHANEL_H

#include <pthread.h>
#include <include/libavcodec/avcodec.h>
#include "JavaCallHelper.h"

class BaseChanel {

public:
    BaseChanel(int chanelId, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext);
    ~BaseChanel();
    int chanelId;
    JavaCallHelper *javaCallHelper;
    pthread_t decode;
    pthread_t syscnsizePlay;
    AVCodecContext *avCodecContext;
    virtual void start() = 0;
    virtual void stop() = 0;
};





#endif //FFMPEG_DIARY_BASECHANEL_H

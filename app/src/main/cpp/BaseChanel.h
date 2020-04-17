//
// Created by kevin on 2020/4/16.
//

#ifndef FFMPEG_DIARY_BASECHANEL_H
#define FFMPEG_DIARY_BASECHANEL_H

#include <pthread.h>

class BaseChanel {

public:
    pthread_t pthreadInit;
    void start();

};





#endif //FFMPEG_DIARY_BASECHANEL_H

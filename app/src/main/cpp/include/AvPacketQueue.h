//
// Created by kevin on 2020/4/13.
//

#ifndef FFMPEG_DIARY_AVPACKETQUEUE_H
#define FFMPEG_DIARY_AVPACKETQUEUE_H

#include <queue>
#include <pthread.h>

extern "C" {
#include "libavcodec/avcodec.h"
};

using namespace std;
class AvPacketQueue {

public:
    AvPacketQueue();

    ~AvPacketQueue();
    queue<AVPacket*> avPacketQueue;
    pthread_cond_t condt;
    pthread_mutex_t mutex;
    pthread_t productThread;
    pthread_t consumerThread;
    bool exitFlag;
    void* pushAvPacket(void *data);
    void* popAvPacket(void *data);

};


#endif //FFMPEG_DIARY_AVPACKETQUEUE_H

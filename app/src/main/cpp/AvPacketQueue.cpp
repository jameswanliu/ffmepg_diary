//
// Created by kevin on 2020/4/13.
//

#include "include/AvPacketQueue.h"

void *AvPacketQueue::pushAvPacket(void *data) {
    while (!exitFlag) {
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&condt);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(&productThread);
}

void *AvPacketQueue::popAvPacket(void *data) {
    while (!exitFlag) {
        pthread_mutex_lock(&mutex);
        if (avPacketQueue.size() > 0) {
            AVPacket *packet = avPacketQueue.back();
            avPacketQueue.pop();
        } else {
            pthread_cond_wait(&condt, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(&consumerThread);
}
AvPacketQueue::AvPacketQueue() {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&condt, nullptr);
    pthread_create(&productThread, NULL,pushAvPacket, NULL);
    pthread_create(&consumerThread, NULL, popAvPacket, NULL);
}


AvPacketQueue::~AvPacketQueue() {
    if (&mutex != NULL) pthread_mutex_destroy(&mutex);
    if (&condt != NULL) pthread_cond_destroy(&condt);
}



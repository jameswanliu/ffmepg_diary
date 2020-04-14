//
// Created by kevin on 2020/4/14.
//

#include "AvPacketQueue.h"
#include "include/StephenController.h"


void StephenController::initFFmpeg() {
    avformat_network_init();
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&condt, nullptr);
}

AVPacket *StephenController::back_pack_to_queue(pthread_mutex_t mutex, pthread_cond_t condt) {

}


void StephenController::push_pack_to_queue(pthread_mutex_t mutex, pthread_cond_t condt,
                                           AVPacket *packet) {
}


StephenController::~StephenController() {
    if (&mutex != NULL) pthread_mutex_destroy(&mutex);
    if (&condt != NULL) pthread_cond_destroy(&condt);
}

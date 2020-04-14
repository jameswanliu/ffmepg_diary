//
// Created by kevin on 2020/4/14.
//

#ifndef FFMPEG_DIARY_STEPHENCONTROLLER_H
#define FFMPEG_DIARY_STEPHENCONTROLLER_H

#include <pthread.h>


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
using namespace std;

class StephenController {

public:

    StephenController();

    ~StephenController();

    AVCodec *video_codec;
    pthread_t productThread;
    pthread_t consumerThread;
    pthread_cond_t condt;
    pthread_mutex_t mutex;
    AVFormatContext *formatContext;
    int audio_stream_index;
    AVCodecContext *video_codec_context;
    void initFFmpeg();

    void push_pack_to_queue(pthread_mutex_t mutex, pthread_cond_t condt, AVPacket *packet);

    AVPacket *back_pack_to_queue(pthread_mutex_t mutex, pthread_cond_t condt);
};


#endif //FFMPEG_DIARY_STEPHENCONTROLLER_H

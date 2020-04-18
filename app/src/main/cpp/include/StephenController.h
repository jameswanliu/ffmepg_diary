//
// Created by kevin on 2020/4/14.
//

#ifndef FFMPEG_DIARY_STEPHENCONTROLLER_H
#define FFMPEG_DIARY_STEPHENCONTROLLER_H

#include <pthread.h>
#include <AudioChanel.h>
#include <VideoChanel.h>


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
using namespace std;


class StephenController {

public:
    StephenController();

    ~StephenController();

    AudioChanel *audioChanel;
    pthread_t pid_create;
    pthread_cond_t condt;
    JavaCallHelper *javaCallHelper;
    const char *url;
    pthread_mutex_t mutex;
    VideoChanel *videoChanel;
    int audio_stream_index;
    AVCodecContext *video_codec_context;

    int prepareFFmpeg();

    void initalFFmpeg(JavaVM *vm, JNIEnv *env, jobject obj,jstring path);
    void startPlay();
};


#endif //FFMPEG_DIARY_STEPHENCONTROLLER_H

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
    void setRenderFrame(RenderFrame renderFrame1);

    int prepareFFmpeg();

    void initalFFmpeg(JavaVM *vm, JNIEnv *env, jobject obj, jstring path);

    void dispatchPacket();

    void start();
private:
    int isPlaying;
    pthread_t pid_create;
    pthread_t pid_dispatch_packet;
    JavaCallHelper *javaCallHelper;
    AudioChanel *audioChanel;
    RenderFrame renderFrame;
    const char *url;
    pthread_mutex_t mutex;
    VideoChanel *videoChanel;
    int index;
    AVCodecContext *video_codec_context;


};


#endif //FFMPEG_DIARY_STEPHENCONTROLLER_H

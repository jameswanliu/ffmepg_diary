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
    StephenController(JavaCallHelper *javaCallHelper);

    ~StephenController(){
        delete[](javaCallHelper,audioChanel,videoChanel,url);
        avformat_free_context(avFormatContext);
    }
    void setRenderFrame(RenderFrame renderFrame1);

    int prepareFFmpeg();

    void initalFFmpeg(JNIEnv *env, jstring path);

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
    AVFormatContext *avFormatContext;

};


#endif //FFMPEG_DIARY_STEPHENCONTROLLER_H

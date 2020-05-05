//
// Created by kevin on 2020/4/16.
//

#ifndef FFMPEG_DIARY_BASECHANEL_H
#define FFMPEG_DIARY_BASECHANEL_H

#include <pthread.h>
#include "safequeue.h"

extern "C" {
#include <include/libavcodec/avcodec.h>
}

#include "JavaCallHelper.h"


/**
 *
 * 音频和视频chanel 继承自BaseChanel
 *
 */
class BaseChanel {

public:
    SafeQueue<AVPacket *> avpacketQueue;
    SafeQueue<AVFrame *> avFrameQueue;

    volatile int chanelId;
    AVRational time_base;
    double clock = 0;
    volatile bool isPlay;
    JavaCallHelper *javaCallHelper;
    pthread_t decode;
    pthread_t syscnsizePlay;
    AVCodecContext *avCodecContext;
    BaseChanel(int chanelId, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,AVRational time_base)
            : chanelId(chanelId), javaCallHelper(javaCallHelper), avCodecContext(avCodecContext),time_base(time_base) {
        avpacketQueue.setReleaseHandle(freeAvPacket);
        avFrameQueue.setReleaseHandle(freeAvFrame);
        avpacketQueue.clear();
        avFrameQueue.clear();
    }

    ~BaseChanel() {
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        avFrameQueue.clear();
        avpacketQueue.clear();
    }



    /**
     *引用传递
     * @param avPacket
     */
    static void freeAvPacket(AVPacket *&avPacket) {
        if (avPacket) {
            av_packet_free(&avPacket);
            avPacket = 0;
        }
    }

    /**
      *引用传递
      * @param avPacket
      */
    static void freeAvFrame(AVFrame *&avFrame) {
        if (avFrame) {
            av_frame_free(&avFrame);
            avFrame = 0;
        }
    }



    virtual void play() = 0;

    virtual void stop() = 0;

private:
};


#endif //FFMPEG_DIARY_BASECHANEL_H

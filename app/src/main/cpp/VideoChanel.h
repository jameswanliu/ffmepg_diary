//
// Created by kevin on 2020/4/18.
//




#include "BaseChanel.h"

#ifndef FFMPEG_DIARY_VIDEOCHANEL_H
#define FFMPEG_DIARY_VIDEOCHANEL_H

/**
 * VideoChanel 作用：将存放入队列中avpacket 取出来 并解码成avframe 然后存入 avframe队列
 */

typedef void (*RenderFrame)(uint8_t *, int, int, int);

class VideoChanel : public BaseChanel {
public:
    VideoChanel(int chanelId, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext);

    ~VideoChanel();

    void decodePacket();

    void setRenderFrame(RenderFrame renderFrame1);

    void sysncFrame();

    virtual void play();

    virtual void stop();

private:
    RenderFrame renderFrame;

};


#endif //FFMPEG_DIARY_VIDEOCHANEL_H

//
// Created by kevin on 2020/4/16.
//

#ifndef FFMPEG_DIARY_AUDIOCHANEL_H
#define FFMPEG_DIARY_AUDIOCHANEL_H

#include "BaseChanel.h"

class AudioChanel : public BaseChanel {
public:
    AudioChanel(int chanelId, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext);

    ~AudioChanel();

    virtual void play();

    virtual void stop();
};


#endif //FFMPEG_DIARY_AUDIOCHANEL_H

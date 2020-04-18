//
// Created by kevin on 2020/4/18.
//

#ifndef FFMPEG_DIARY_VIDEOCHANEL_H
#define FFMPEG_DIARY_VIDEOCHANEL_H


class VideoChanel : BaseChanel {
public:
    VideoChanel(int chanelId, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext)
            : BaseChanel(chanelId, javaCallHelper, avCodecContext) {

    };

    ~VideoChanel();

    virtual void start();

    virtual void stop();

};


#endif //FFMPEG_DIARY_VIDEOCHANEL_H

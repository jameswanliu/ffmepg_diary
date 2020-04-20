//
// Created by kevin on 2020/4/18.
//


#include <include/androidlog.h>
#include "VideoChanel.h"
#include "JavaCallHelper.h"

extern "C" {
#include <include/libavutil/time.h>
#include "include/libavcodec/avcodec.h"
#include <include/libavutil/imgutils.h>
#include <include/libswscale/swscale.h>
}

#include "BaseChanel.h"

/**
 * VideoChanel 作用：将存放入队列中avpacket 取出来 并解码成avframe 然后存入 avframe队列
 */
VideoChanel::VideoChanel(int chanelId, JavaCallHelper *javaCallHelper,
                         AVCodecContext *avCodecContext) : BaseChanel(chanelId, javaCallHelper,
                                                                      avCodecContext) {

}


VideoChanel::~VideoChanel() {

}


void *sysncFrameThread(void *args) {
    VideoChanel *videoChanel = static_cast<VideoChanel *>(args);
    videoChanel->sysncFrame();
    return 0;
}


void VideoChanel::sysncFrame() {
    int ret = -1;
    uint8_t *pointers[0];
    int linesizes[0];

    //实例化转换上下文
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt,
                                            avCodecContext->width, avCodecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);

    //分配大小为w和h、像素格式为pix_fmt的图像，
    // 以及相应地填充指针和线条大小。必须使用 av_freep(&pointers[0]) 释放内存
    av_image_alloc(pointers, linesizes, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    AVFrame *avFrame = NULL;
    while (isPlay) {
        ret = avFrameQueue.deQueue(avFrame);
        if (!isPlay) {
            break;
        }
        if (!ret) {
            continue;
        }
        // 通过swscontext 将yuv 数据转换成rgb 数据 注意 avframe 通过receive_frame已经获取到了数据 ，
        // 输入源是frame->data,pointer 是通过av_image_alloc
        //申请的格式为AV_PIX_FMT_RGBA 格式的图像，linesizes是申请的行数，所以 pointer 是 输出源
        sws_scale(swsContext, (const uint8_t *const *) (avFrame->data),
                  avFrame->linesize, 0, avFrame->height, pointers,
                  linesizes);
        renderFrame(pointers[0], linesizes[0], avCodecContext->width, avCodecContext->height);
        av_usleep(16 * 1000000);
        freeAvFrame(avFrame);
    }


    isPlay = 0;
    av_free(pointers);
    freeAvFrame(avFrame);
    sws_freeContext(swsContext);
}


void *decodeThread(void *args) {
    VideoChanel *videoChanel = static_cast<VideoChanel *>(args);
    videoChanel->decodePacket();
    return 0;
}


void VideoChanel::decodePacket() {
    int ret;
    AVPacket *avPacket = NULL;
    AVFrame *frame = NULL;
    while (isPlay) {
        if (!isPlay) {
            break;
        }
        ret = avpacketQueue.deQueue(avPacket);

        if (ret != 0) {
            continue;
        }

        if (avFrameQueue.size() > 100) {
            av_usleep(1000 * 10);
            continue;
        }

        ret = avcodec_send_packet(avCodecContext, avPacket);
        freeAvPacket(avPacket);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }
        frame = av_frame_alloc();
        avcodec_receive_frame(avCodecContext, frame);
        avFrameQueue.enQueue(frame);
        while (isPlay && avFrameQueue.size() > 100) {
            av_usleep(1000 * 10);
            continue;
        }
//        freeAvFrame(frame);
    }
    freeAvPacket(avPacket);
//    freeAvFrame(frame);
}


void VideoChanel::start() {
    avpacketQueue.setWork(1);
    avFrameQueue.setWork(1);
    isPlay = 1;
    pthread_create(&decode, NULL, decodeThread, this);
    pthread_create(&syscnsizePlay, NULL, sysncFrameThread, this);
}


void VideoChanel::stop() {
    avpacketQueue.setWork(0);
    avFrameQueue.setWork(0);
    isPlay = 0;
}

void VideoChanel::setRenderFrame(RenderFrame renderFrame1) {
    this->renderFrame = renderFrame1;
}
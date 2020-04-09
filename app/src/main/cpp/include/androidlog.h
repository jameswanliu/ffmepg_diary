//
// Created by kevin on 2020/4/9.
//

#ifndef FFMPEG_DIARY_ANDROIDLOG_H
#define FFMPEG_DIARY_ANDROIDLOG_H
#endif //FFMPEG_DIARY_ANDROIDLOG_H
#include <android/log.h>

#define LOGI(TAG, FORMAT, ...) __android_log_print(ANDROID_LOG_INFO, TAG, FORMAT, ##__VA_ARGS__);
#define LOGE(TAG, FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, TAG, FORMAT, ##__VA_ARGS__);

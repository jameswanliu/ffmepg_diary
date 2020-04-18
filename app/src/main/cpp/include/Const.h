//
// Created by kevin on 2020/4/18.
//

#ifndef FFMPEG_DIARY_CONST_H
#define FFMPEG_DIARY_CONST_H

#define delete(obj) if(obj){delete(obj); obj = 0;}


#define THREAD_MAIN 1

#define THREAD_CHILD 2

#define FFMPEG_CAN_NOT_OPEN_FILE 1

#define FFMPEG_CAN_NOT_FIND_STREAM 2

#define FFMPEG_FIND_DECODER_FAIL 3

#define FFMPEG_ALLOC_DECODE_CONTEXT_FAIL  4

#define FFMPEG_DECODE_PARAMS_CONTEXT_FAIL 5

#define FFMPEG_OPEN_DECODER_FAIL 6

#define FFMPEG_NOMEDIA 7


#endif //FFMPEG_DIARY_CONST_H

package com.jamestony.ffmpeg_diary.version

/**
 * create by stephen
 * on 2020/4/10
 */


class Version {
    companion object{
        init {
            System.loadLibrary("StephenFFmpeg")
        }
    }
    external fun getVersion(): String
}
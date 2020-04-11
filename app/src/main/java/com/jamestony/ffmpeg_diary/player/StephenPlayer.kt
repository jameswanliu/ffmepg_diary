package com.jamestony.ffmpeg_diary.player

import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

/**
 * create by stephen
 * on 2020/4/9
 */


class StephenPlayer(surfaceView: SurfaceView) : SurfaceHolder.Callback {

    lateinit var surfaceHolder: SurfaceHolder

    external fun play(path: String,surface: Surface):Int

    fun startPlay(path: String){
        play(path,surfaceHolder.surface)
    }

    init {
        System.loadLibrary("native-lib")
        surfaceHolder = surfaceView.holder
        surfaceHolder.addCallback(this)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        surfaceHolder = holder
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
    }

    override fun surfaceCreated(holder: SurfaceHolder) {

    }
}
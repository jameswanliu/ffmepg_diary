package com.jamestony.ffmpeg_diary.player

import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

/**
 * create by stephen
 * on 2020/4/9
 */


class StephenPlayer(private val surfaceView: SurfaceView) : SurfaceHolder.Callback {

    lateinit var surface: Surface

    external fun play(path: String,surface: Surface):Int



    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    init {
        surfaceView.holder.addCallback(this)
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        surface = holder.surface
    }
}
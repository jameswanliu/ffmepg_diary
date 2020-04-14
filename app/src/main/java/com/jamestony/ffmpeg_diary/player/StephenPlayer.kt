package com.jamestony.ffmpeg_diary.player

import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import io.reactivex.Flowable
import io.reactivex.disposables.Disposable
import io.reactivex.schedulers.Schedulers
import org.reactivestreams.Subscription

/**
 * create by stephen
 * on 2020/4/9
 */


class StephenPlayer(surfaceView: SurfaceView) : SurfaceHolder.Callback {

    lateinit var surfaceHolder: SurfaceHolder

    external fun playVideo(path: String, surface: Surface): Int
    var subscriberList = arrayListOf<Disposable>()

    fun startPlay(path: String) {
        subscriberList.add(Flowable.just(path).subscribeOn(Schedulers.io()).subscribe { s ->
            playVideo(s, surfaceHolder.surface)
        })
    }

    init {
        System.loadLibrary("StephenFFmpeg")
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

    fun onRelease(){
        subscriberList.forEach {
            it.dispose()
        }
    }
}
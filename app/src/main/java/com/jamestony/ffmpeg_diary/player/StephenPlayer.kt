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


class StephenPlayer(
    val surfaceView: SurfaceView,
    val onPrepare: () -> Unit,
    val onError: (Int) -> Unit,
    val onProgress: (Int) -> Unit
) : SurfaceHolder.Callback {

    lateinit var surfaceHolder: SurfaceHolder

    external fun playVideo(path: String, surface: Surface): Int

    external fun native_setSurface(surface: Surface = surfaceHolder.surface)
    external fun native_video_prepare(path: String): Int
    external fun native_start()
    external fun native_initial()

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

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) =Unit

    fun prepare() {
        onPrepare()
    }


    fun error(code: Int) {
        onError(code)
    }

    fun progress(progress: Int) {
        onProgress(progress)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) = Unit
    override fun surfaceCreated(holder: SurfaceHolder) {
        surfaceHolder = holder
    }

    fun onRelease() {
        subscriberList.forEach {
            it.dispose()
        }
    }
}
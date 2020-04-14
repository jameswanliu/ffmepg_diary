package com.jamestony.ffmpeg_diary.player

import android.view.Surface
import io.reactivex.Flowable
import io.reactivex.disposables.Disposable
import io.reactivex.schedulers.Schedulers

/**
 * create by stephen
 * on 2020/4/14
 */
class SteAudioPlayer {
    companion object{
        init {
            System.loadLibrary("StephenFFmpeg")
        }
    }
    external fun playAudio(path: String,output:String): Int
    var subscriberList = arrayListOf<Disposable>()

    fun startPlay(path: String,output:String) {
        subscriberList.add(Flowable.just(path).subscribeOn(Schedulers.io()).subscribe { s ->
            playAudio(s,output)
        })
    }

    fun onRelease(){
        subscriberList.forEach {
            it.dispose()
        }
    }
}
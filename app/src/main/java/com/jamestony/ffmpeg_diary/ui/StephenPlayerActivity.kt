package com.jamestony.ffmpeg_diary.ui

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.jamestony.ffmpeg_diary.R
import com.jamestony.ffmpeg_diary.application.StephenApplication.Companion.ExternalStorageDirectoryPath
import com.jamestony.ffmpeg_diary.player.StephenPlayer
import kotlinx.android.synthetic.main.activity_stephen_player.*

class StephenPlayerActivity : AppCompatActivity() {

    lateinit var stephenPlayer: StephenPlayer


    private val music = ExternalStorageDirectoryPath + "tencent/qqfile_recv/Love.mp3"
    private val video =
        ExternalStorageDirectoryPath + "VBrowserData/3a544f6ffb69431b8d7d36b76c623e51.mp4/video.mp4"


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_stephen_player)
        stephenPlayer = StephenPlayer(surface, ::onPrepare, ::onError, ::onProgress)
        stephenPlayer.native_initial()
        val ret = stephenPlayer.native_video_prepare(video)
        Log.i("ret", "ret = $ret")
        acb.setOnClickListener {
            stephenPlayer.native_setSurface()
            stephenPlayer.native_start()
        }
    }


    fun onPrepare() {
        Log.i("onPrepare", "onPrepare success")
    }


    fun onError(code: Int) {
        Log.i("onError", "code=${code}")
    }


    fun onProgress(progress: Int) {
        Log.i("onProgress", "progress=${progress}")
    }
}
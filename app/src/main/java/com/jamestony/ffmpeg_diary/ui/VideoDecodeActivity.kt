package com.jamestony.ffmpeg_diary.ui

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.jamestony.ffmpeg_diary.R
import com.jamestony.ffmpeg_diary.application.StephenApplication.Companion.ExternalStorageDirectoryPath
import com.jamestony.ffmpeg_diary.player.StephenPlayer
import kotlinx.android.synthetic.main.activity_video_decode.*

/**
 * create by stephen
 * on 2020/4/13
 */
class VideoDecodeActivity : AppCompatActivity() {

    lateinit var stephenPlayer: StephenPlayer
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_video_decode)
        stephenPlayer = StephenPlayer(surface, ::onPrepare, ::onError, ::onProgress)
        acb.setOnClickListener {
            stephenPlayer.startPlay(ExternalStorageDirectoryPath + "VBrowserData/3a544f6ffb69431b8d7d36b76c623e51.mp4/video.mp4")
        }
    }


    fun onProgress(progress: Int) {

    }


    fun onError(code: Int) {

    }


    fun onPrepare() {

    }

    override fun onDestroy() {
        stephenPlayer.onRelease()
        super.onDestroy()
    }
}
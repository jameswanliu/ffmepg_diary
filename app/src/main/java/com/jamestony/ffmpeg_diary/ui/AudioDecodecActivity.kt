package com.jamestony.ffmpeg_diary.ui

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.jamestony.ffmpeg_diary.R
import com.jamestony.ffmpeg_diary.application.StephenApplication
import com.jamestony.ffmpeg_diary.player.SteAudioPlayer
import kotlinx.android.synthetic.main.activity_audio_decode.*

class AudioDecodecActivity : AppCompatActivity() {

    lateinit var audioPlayer: SteAudioPlayer
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_audio_decode)
        audioPlayer = SteAudioPlayer()
        apc_start.setOnClickListener {
            audioPlayer.startPlay(StephenApplication.ExternalStorageDirectoryPath +
                    "tencent/qqfile_recv/Love.mp3", StephenApplication.ExternalStorageDirectoryPath + "tencent/qqfile_recv/Love.pcm")
        }
    }
}
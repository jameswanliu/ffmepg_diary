package com.jamestony.ffmpeg_diary.ui

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.jamestony.ffmpeg_diary.MainActivity
import com.jamestony.ffmpeg_diary.R
import com.jamestony.ffmpeg_diary.player.StephenPlayer
import com.jamestony.ffmpeg_diary.version.Version
import kotlinx.android.synthetic.main.activity_splash.*

/**
 * create by stephen
 * on 2020/4/10
 */

class SplashActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_splash)
        sample_text.text = "FFMPEG:\t${Version().getVersion()}"
        sample_text.postDelayed({
            startActivity(Intent(this,MainActivity::class.java))
            finish()
        },2000)
    }
}
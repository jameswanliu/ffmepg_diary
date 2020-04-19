package com.jamestony.ffmpeg_diary

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.hjq.permissions.OnPermission
import com.hjq.permissions.Permission
import com.hjq.permissions.XXPermissions
import com.jamestony.ffmpeg_diary.ui.AudioDecodecActivity
import com.jamestony.ffmpeg_diary.ui.StephenPlayerActivity
import com.jamestony.ffmpeg_diary.ui.VideoDecodeActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        XXPermissions.with(this)
            .permission(
                Permission.Group.STORAGE
            )
            .request(object : OnPermission {
                override fun hasPermission(
                    granted: List<String>,
                    isAll: Boolean
                ) = Unit

                override fun noPermission(
                    denied: List<String>,
                    quick: Boolean
                ) = Unit
            })

        acb_audio_decode.setOnClickListener {
            startActivity(Intent(this, AudioDecodecActivity::class.java))
        }

        acb_video_decode.setOnClickListener {
            startActivity(Intent(this, VideoDecodeActivity::class.java))
        }

        acb_stephen.setOnClickListener {
            startActivity(Intent(this, StephenPlayerActivity::class.java))
        }
    }
}

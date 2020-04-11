package com.jamestony.ffmpeg_diary

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import com.hjq.permissions.OnPermission
import com.hjq.permissions.Permission
import com.hjq.permissions.XXPermissions
import com.jamestony.ffmpeg_diary.player.StephenPlayer
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File

class MainActivity : AppCompatActivity() {
    lateinit var stephenPlayer: StephenPlayer
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        stephenPlayer = StephenPlayer(surface)
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

        acb.setOnClickListener {
            stephenPlayer.startPlay(Environment.getExternalStorageDirectory().path + File.separator + "VBrowserData/3a544f6ffb69431b8d7d36b76c623e51.mp4/video.mp4")
        }
    }
}

package com.jamestony.ffmpeg_diary.application

import android.app.Application
import android.content.ContextWrapper
import android.os.Environment
import java.io.File

/**
 * create by stephen
 * on 2020/4/13
 */


lateinit var INTANCE: StephenApplication

class StephenApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        INTANCE = this
    }

    companion object{
       val ExternalStorageDirectoryPath  = Environment.getExternalStorageDirectory().path + File.separator
    }
}
object applicationContext : ContextWrapper(INTANCE)

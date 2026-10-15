package com.haiseiko.skeaclock

import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.appcompat.app.AppCompatActivity
import android.content.Context
import android.view.Surface

class MainActivity : AppCompatActivity() {
    external fun getTimeAngles(): FloatArray
    external fun nativeIsVulkanAvailable(): Boolean
    external fun nativeInitVulkan(surface: Surface)
    external fun nativeRenderFrame()
    external fun nativeShutdownVulkan()

    companion object {
        init { System.loadLibrary("native-lib") }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val view = ClockView(this)
        setContentView(view)
    }

    private inner class ClockView(context: Context) : SurfaceView(context), SurfaceHolder.Callback, Runnable {
        private var thread: Thread? = null
        private var running = false

        init { holder.addCallback(this) }

        override fun surfaceCreated(holder: SurfaceHolder) {
            running = true
            val supported = nativeIsVulkanAvailable()
            if (supported) {
                nativeInitVulkan(holder.surface)
            }
            thread = Thread(this)
            thread?.start()
        }

        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            running = false
            nativeShutdownVulkan()
            try { thread?.join() } catch (_: InterruptedException) { }
        }

        override fun run() {
            while (running) {
                if (nativeIsVulkanAvailable()) {
                    nativeRenderFrame()
                }
                try { Thread.sleep(16) } catch (_: InterruptedException) { }
            }
        }
    }
}

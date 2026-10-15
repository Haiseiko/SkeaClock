package com.haiseiko.skeaclock

import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.appcompat.app.AppCompatActivity
import android.content.Context
import android.graphics.*

class MainActivity : AppCompatActivity() {
    external fun getTimeAngles(): FloatArray

    companion object {
        init { System.loadLibrary("native-lib") }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val view = ClockView(this)
        setContentView(view)
    }

    inner class ClockView(context: Context) : SurfaceView(context), SurfaceHolder.Callback, Runnable {
        private var thread: Thread? = null
        private var running = false

        init { holder.addCallback(this) }

        override fun surfaceCreated(holder: SurfaceHolder) {
            running = true
            thread = Thread(this); thread?.start()
        }

        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            running = false
            try { thread?.join() } catch (e: InterruptedException) { /* ignore */ }
        }

        override fun run() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG)
            while (running) {
                val canvas = holder.lockCanvas()
                if (canvas != null) {
                    canvas.drawColor(Color.WHITE)
                    val w = canvas.width; val h = canvas.height
                    val cx = w / 2f; val cy = h / 2f
                    val radius = Math.min(cx, cy) * 0.9f
                    val angles = getTimeAngles()

                    // Draw clock face
                    paint.style = Paint.Style.STROKE
                    paint.strokeWidth = 8f
                    paint.color = Color.BLACK
                    canvas.drawCircle(cx, cy, radius, paint)

                    // Draw hands
                    paint.style = Paint.Style.STROKE
                    paint.strokeWidth = 6f
                    paint.color = Color.BLACK
                    val hourAngle = angles[0] * 360f - 90f
                    val minuteAngle = angles[1] * 360f - 90f
                    val secondAngle = angles[2] * 360f - 90f

                    fun drawHand(angle: Float, len: Float) {
                        val rad = Math.toRadians(angle.toDouble())
                        val x = cx + Math.cos(rad).toFloat() * radius * len
                        val y = cy + Math.sin(rad).toFloat() * radius * len
                        canvas.drawLine(cx, cy, x, y, paint)
                    }

                    drawHand(hourAngle, 0.5f)
                    paint.strokeWidth = 4f
                    drawHand(minuteAngle, 0.8f)
                    paint.strokeWidth = 2f
                    paint.color = Color.RED
                    drawHand(secondAngle, 0.9f)

                    holder.unlockCanvasAndPost(canvas)
                }
                try { Thread.sleep(16) } catch (e: InterruptedException) { /* ignore */ }
            }
        }
    }
}

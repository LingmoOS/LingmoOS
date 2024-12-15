// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

import android.annotation.SuppressLint
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.content.pm.ServiceInfo
import android.content.res.Configuration
import android.graphics.Bitmap
import android.graphics.PixelFormat
import android.hardware.display.DisplayManager
import android.hardware.display.VirtualDisplay
import android.media.ImageReader
import android.media.projection.MediaProjection
import android.media.projection.MediaProjectionManager
import android.os.Build
import android.os.IBinder
import android.util.Log
import android.view.Display
import androidx.preference.PreferenceManager
import com.lingmo.assistant.R
import com.lingmo.assistant.services.MainService.Companion.currentNotification
import com.lingmo.assistant.services.MainService.Companion.vncGetFramebufferHeight
import com.lingmo.assistant.services.MainService.Companion.vncGetFramebufferWidth
import com.lingmo.assistant.services.MainService.Companion.vncNewFramebuffer
import com.lingmo.assistant.services.MainService.Companion.vncUpdateFramebuffer
import com.lingmo.assistant.services.MediaProjectionRequestActivity
import com.lingmo.assistant.utils.Utils.getDisplayMetrics
import java.nio.ByteBuffer

class MediaProjectionService : Service() {
    private var mResultCode = 0
    private var mResultData: Intent? = null
    private var mImageReader: ImageReader? = null
    private var mVirtualDisplay: VirtualDisplay? = null
    private var mMediaProjection: MediaProjection? = null
    private var mMediaProjectionManager: MediaProjectionManager? = null

    private var mHasPortraitInLandscapeWorkaroundApplied = false
    private var mHasPortraitInLandscapeWorkaroundSet = false

    override fun onBind(intent: Intent): IBinder? {
        return null
    }

    override fun onCreate() {
        Log.d(TAG, "onCreate")

        instance = this

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            /*
                Create notification channel
             */
            val serviceChannel = NotificationChannel(
                packageName,
                "Foreground Service Channel",
                NotificationManager.IMPORTANCE_DEFAULT
            )
            val manager = getSystemService(
                NotificationManager::class.java
            )
            manager.createNotificationChannel(serviceChannel)

            /*
                startForeground() w/ notification; bit hacky re-using MainService's ;-)
             */
            try {
                if (Build.VERSION.SDK_INT >= 29) {
                    // throws NullPointerException if no notification
                    currentNotification?.let {
                        startForeground(
                            MainService.NOTIFICATION_ID,
                            it,
                            ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PROJECTION
                        )
                    }
                } else {
                    // throws IllegalArgumentException if no notification
                    startForeground(MainService.NOTIFICATION_ID, currentNotification)
                }
            } catch (ignored: Exception) {
                Log.e(TAG, "Not starting because MainService quit")
            }
        }

        /*
            Get the MediaProjectionManager
         */
        mMediaProjectionManager =
            getSystemService(MEDIA_PROJECTION_SERVICE) as MediaProjectionManager
    }


    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)

        val displayMetrics = getDisplayMetrics(this, Display.DEFAULT_DISPLAY)
        Log.d(TAG, "onConfigurationChanged: width: " + displayMetrics.widthPixels + " height: " + displayMetrics.heightPixels)

        startScreenCapture()
    }


    override fun onDestroy() {
        Log.d(TAG, "onDestroy")

        stopScreenCapture()

        instance = null
    }


    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        mResultCode = intent.getIntExtra(MainService.EXTRA_MEDIA_PROJECTION_RESULT_CODE, 0)
        mResultData = intent.getParcelableExtra(MainService.EXTRA_MEDIA_PROJECTION_RESULT_DATA)

        startScreenCapture()

        return START_REDELIVER_INTENT
    }


    @SuppressLint("WrongConstant")
    private fun startScreenCapture() {
        if (mMediaProjection == null) try {
            mMediaProjection =
                mMediaProjectionManager!!.getMediaProjection(mResultCode, mResultData!!)
        } catch (e: SecurityException) {
            Log.w(TAG, "startScreenCapture: got SecurityException, re-requesting confirmation")
            // This initiates a prompt dialog for the user to confirm screen projection.
            val mediaProjectionRequestIntent =
                Intent(this, MediaProjectionRequestActivity::class.java)
            mediaProjectionRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            startActivity(mediaProjectionRequestIntent)
            return
        }

        if (mMediaProjection == null) {
            Log.e(TAG, "startScreenCapture: did not get a media projection, probably user denied")
            return
        }

        // Android 14 and newer require this callback
        mMediaProjection!!.registerCallback(
            object : MediaProjection.Callback() {
                override fun onStop() {
                    Log.d(TAG, "callback: onStop")
                    super.onStop()
                }

                override fun onCapturedContentResize(width: Int, height: Int) {
                    Log.d(TAG, "callback: onCapturedContentResize " + width + "x" + height)
                }

                override fun onCapturedContentVisibilityChanged(isVisible: Boolean) {
                    Log.d(TAG, "callback: onCapturedContentVisibilityChanged $isVisible")
                }
            },
            null
        )

        if (mImageReader != null) mImageReader!!.close()

        val metrics = getDisplayMetrics(this, Display.DEFAULT_DISPLAY)

        // apply selected scaling
        val scaling = PreferenceManager.getDefaultSharedPreferences(this).getFloat(
            Constants.PREFS_KEY_SERVER_LAST_SCALING, Defaults(
                this
            ).scaling
        )
        val scaledWidth = (metrics.widthPixels * scaling).toInt()
        val scaledHeight = (metrics.heightPixels * scaling).toInt()

        // only set this by detecting quirky hardware if the user has not set manually
        if (!mHasPortraitInLandscapeWorkaroundSet && Build.FINGERPRINT.contains("rk3288") && metrics.widthPixels > 800) {
            Log.w(TAG, "detected >10in rk3288 applying workaround for portrait-in-landscape quirk")
            mHasPortraitInLandscapeWorkaroundApplied = true
        }

        // use workaround if flag set and in actual portrait mode
        if (mHasPortraitInLandscapeWorkaroundApplied && scaledWidth < scaledHeight) {
            val portraitInsideLandscapeScaleFactor = scaledWidth.toFloat() / scaledHeight

            // width and height are swapped here
            val quirkyLandscapeWidth =
                (scaledHeight.toFloat() / portraitInsideLandscapeScaleFactor).toInt()
            val quirkyLandscapeHeight =
                (scaledWidth.toFloat() / portraitInsideLandscapeScaleFactor).toInt()

            mImageReader = ImageReader.newInstance(
                quirkyLandscapeWidth,
                quirkyLandscapeHeight,
                PixelFormat.RGBA_8888,
                2
            )
            mImageReader!!.setOnImageAvailableListener({ imageReader: ImageReader ->
                try {
                    imageReader.acquireLatestImage().use { image ->
                        if (image == null) return@setOnImageAvailableListener
                        val planes = image.planes
                        val buffer = planes[0].buffer
                        val pixelStride = planes[0].pixelStride
                        val rowStride = planes[0].rowStride
                        val rowPadding = rowStride - pixelStride * quirkyLandscapeWidth
                        val w = quirkyLandscapeWidth + rowPadding / pixelStride

                        // create destination Bitmap
                        val dest =
                            Bitmap.createBitmap(w, quirkyLandscapeHeight, Bitmap.Config.ARGB_8888)

                        // copy landscape buffer to dest bitmap
                        buffer.rewind()
                        dest.copyPixelsFromBuffer(buffer)

                        // get the portrait portion that's in the center of the landscape bitmap
                        val croppedDest = Bitmap.createBitmap(
                            dest,
                            quirkyLandscapeWidth / 2 - scaledWidth / 2,
                            0,
                            scaledWidth,
                            scaledHeight
                        )

                        val croppedBuffer =
                            ByteBuffer.allocateDirect(scaledWidth * scaledHeight * 4)
                        croppedDest.copyPixelsToBuffer(croppedBuffer)

                        // if needed, setup a new VNC framebuffer that matches the new buffer's dimensions
                        if (scaledWidth != vncGetFramebufferWidth() || scaledHeight != vncGetFramebufferHeight()) vncNewFramebuffer(
                            scaledWidth,
                            scaledHeight
                        )
                        vncUpdateFramebuffer(croppedBuffer)
                    }
                } catch (ignored: Exception) {
                }
            }, null)

            try {
                if (mVirtualDisplay == null) {
                    mVirtualDisplay = mMediaProjection!!.createVirtualDisplay(
                        getString(R.string.app_name),
                        quirkyLandscapeWidth, quirkyLandscapeHeight, metrics.densityDpi,
                        DisplayManager.VIRTUAL_DISPLAY_FLAG_AUTO_MIRROR,
                        mImageReader!!.surface, null, null
                    )
                } else {
                    mVirtualDisplay!!.resize(
                        quirkyLandscapeWidth,
                        quirkyLandscapeHeight,
                        metrics.densityDpi
                    )
                    mVirtualDisplay!!.surface = mImageReader!!.surface
                }
            } catch (e: SecurityException) {
                Log.w(TAG, "startScreenCapture: got SecurityException, re-requesting confirmation")
                // This initiates a prompt dialog for the user to confirm screen projection.
                val mediaProjectionRequestIntent =
                    Intent(this, MediaProjectionRequestActivity::class.java)
                mediaProjectionRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                startActivity(mediaProjectionRequestIntent)
            }

            return
        }

        /*
            This is the default behaviour.
         */
        mImageReader = ImageReader.newInstance(scaledWidth, scaledHeight, PixelFormat.RGBA_8888, 2)
        mImageReader!!.setOnImageAvailableListener({ imageReader: ImageReader ->
            try {
                imageReader.acquireLatestImage().use { image ->
                    if (image == null) return@setOnImageAvailableListener
                    val planes = image.planes
                    val buffer = planes[0].buffer
                    val pixelStride = planes[0].pixelStride
                    val rowStride = planes[0].rowStride
                    val rowPadding = rowStride - pixelStride * scaledWidth
                    val w = scaledWidth + rowPadding / pixelStride

                    // if needed, setup a new VNC framebuffer that matches the image plane's parameters
                    if (w != vncGetFramebufferWidth() || scaledHeight != vncGetFramebufferHeight()) {
                        vncNewFramebuffer(w, scaledHeight)
                    }

                    buffer.rewind()
                    vncUpdateFramebuffer(buffer)
                }
            } catch (ignored: Exception) {
            }
        }, null)

        try {
            if (mVirtualDisplay == null) {
                mVirtualDisplay = mMediaProjection!!.createVirtualDisplay(
                    getString(R.string.app_name),
                    scaledWidth, scaledHeight, metrics.densityDpi,
                    DisplayManager.VIRTUAL_DISPLAY_FLAG_AUTO_MIRROR,
                    mImageReader!!.surface, null, null
                )
            } else {
                mVirtualDisplay!!.resize(scaledWidth, scaledHeight, metrics.densityDpi)
                mVirtualDisplay!!.surface = mImageReader!!.surface
            }
        } catch (e: SecurityException) {
            Log.w(TAG, "startScreenCapture: got SecurityException, re-requesting confirmation")
            // This initiates a prompt dialog for the user to confirm screen projection.
            val mediaProjectionRequestIntent =
                Intent(this, MediaProjectionRequestActivity::class.java)
            mediaProjectionRequestIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            startActivity(mediaProjectionRequestIntent)
        }
    }

    private fun stopScreenCapture() {
        try {
            mVirtualDisplay!!.release()
            mVirtualDisplay = null
        } catch (e: Exception) {
            //unused
        }

        if (mMediaProjection != null) {
            mMediaProjection!!.stop()
            mMediaProjection = null
        }
    }

    companion object {
        private const val TAG = "MediaProjectionService"

        private var instance: MediaProjectionService? = null


        /**
         * Get whether Media Projection was granted by the user.
         */
        fun isMediaProjectionEnabled(): Boolean {
            return instance != null && instance!!.mResultCode != 0 && instance!!.mResultData != null
        }

        @JvmStatic
        fun togglePortraitInLandscapeWorkaround() {
            try {
                // set
                instance!!.mHasPortraitInLandscapeWorkaroundSet = true
                instance!!.mHasPortraitInLandscapeWorkaroundApplied =
                    !instance!!.mHasPortraitInLandscapeWorkaroundApplied
                Log.d(TAG, "togglePortraitInLandscapeWorkaround: now " + instance!!.mHasPortraitInLandscapeWorkaroundApplied)
                // apply
                instance!!.startScreenCapture()
            } catch (e: NullPointerException) {
                //unused
            }
        }
    }
}

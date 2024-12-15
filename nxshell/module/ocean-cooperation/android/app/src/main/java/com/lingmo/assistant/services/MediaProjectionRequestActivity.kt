// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

import android.app.AlertDialog
import android.content.DialogInterface
import android.content.Intent
import android.media.projection.MediaProjectionManager
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.PreferenceManager
import com.lingmo.assistant.R
import com.lingmo.assistant.services.MainService

class MediaProjectionRequestActivity : AppCompatActivity() {
    private var mIsUpgradingFromFallbackScreenCapture = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        mIsUpgradingFromFallbackScreenCapture = intent.getBooleanExtra(
            EXTRA_UPGRADING_FROM_FALLBACK_SCREEN_CAPTURE, false
        )

        val mMediaProjectionManager =
            getSystemService(MEDIA_PROJECTION_SERVICE) as MediaProjectionManager

        if (!mIsUpgradingFromFallbackScreenCapture) {
            // ask for MediaProjection right away
            Log.i(TAG, "Requesting confirmation")
            // This initiates a prompt dialog for the user to confirm screen projection.
            startActivityForResult(
                mMediaProjectionManager.createScreenCaptureIntent(),
                REQUEST_MEDIA_PROJECTION
            )
        } else {
            // show user info dialog before asking
            AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.mediaprojection_request_activity_fallback_screen_capture_title)
                .setMessage(R.string.mediaprojection_request_activity_fallback_screen_capture_msg)
                .setPositiveButton(R.string.yes) { dialog: DialogInterface?, which: Int ->
                    // This initiates a prompt dialog for the user to confirm screen projection.
                    startActivityForResult(
                        mMediaProjectionManager.createScreenCaptureIntent(),
                        REQUEST_MEDIA_PROJECTION
                    )
                }
                .setNegativeButton(getString(R.string.no)) { dialog: DialogInterface?, which: Int -> finish() }
                .show()
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)

        if (requestCode == REQUEST_MEDIA_PROJECTION) {
            if (resultCode != RESULT_OK) Log.i(TAG, "User cancelled")
            else Log.i(TAG, "User acknowledged")

            val intent = Intent(this, MainService::class.java)
            intent.setAction(MainService.ACTION_HANDLE_MEDIA_PROJECTION_RESULT)
            intent.putExtra(
                MainService.EXTRA_ACCESS_KEY, PreferenceManager.getDefaultSharedPreferences(
                    this
                ).getString(
                    Constants.PREFS_KEY_SETTINGS_ACCESS_KEY, Defaults(
                        this
                    ).accessKey
                )
            )
            intent.putExtra(MainService.EXTRA_MEDIA_PROJECTION_RESULT_CODE, resultCode)
            intent.putExtra(MainService.EXTRA_MEDIA_PROJECTION_RESULT_DATA, data)
            intent.putExtra(
                MainService.EXTRA_MEDIA_PROJECTION_UPGRADING_FROM_FALLBACK_SCREEN_CAPTURE,
                mIsUpgradingFromFallbackScreenCapture
            )
            startService(intent)
            finish()
        }
    }

    companion object {
        private const val TAG = "MPRequestActivity"
        private const val REQUEST_MEDIA_PROJECTION = 42
        const val EXTRA_UPGRADING_FROM_FALLBACK_SCREEN_CAPTURE: String =
            "upgrading_from_fallback_screen_capture"
    }
}
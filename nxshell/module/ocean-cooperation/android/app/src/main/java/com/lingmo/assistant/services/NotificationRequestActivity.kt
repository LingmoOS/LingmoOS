// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

import android.Manifest
import android.app.AlertDialog
import android.content.DialogInterface
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.PreferenceManager
import com.lingmo.assistant.R
import com.lingmo.assistant.services.MainService

class NotificationRequestActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Build.VERSION.SDK_INT < 33) {
            // no permission needed on API level < 33
            postResultAndFinish(true)
            return
        }

        if (checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            Log.i(TAG, "Has no permission! Ask!")

            val prefs = PreferenceManager.getDefaultSharedPreferences(this)

            if (!prefs.getBoolean(PREFS_KEY_POST_NOTIFICATION_PERMISSION_ASKED_BEFORE, false)) {
                AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setTitle(R.string.notification_title)
                    .setMessage(R.string.notification_msg)
                    .setPositiveButton(R.string.yes) { dialog: DialogInterface?, which: Int ->
                        requestPermissions(
                            arrayOf(Manifest.permission.POST_NOTIFICATIONS),
                            REQUEST_POST_NOTIFICATION
                        )
                    }
                    .setNegativeButton(getString(R.string.no)) { dialog: DialogInterface?, which: Int ->
                        postResultAndFinish(
                            false
                        )
                    }
                    .show()
                val ed = prefs.edit()
                ed.putBoolean(PREFS_KEY_POST_NOTIFICATION_PERMISSION_ASKED_BEFORE, true)
                ed.apply()
            } else {
                postResultAndFinish(false)
            }
        } else {
            Log.i(TAG, "Permission already given!")
            postResultAndFinish(true)
        }
    }


    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)

        if (requestCode == REQUEST_POST_NOTIFICATION) {
            postResultAndFinish(grantResults.size > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
        }
    }

    private fun postResultAndFinish(isPermissionGiven: Boolean) {
        if (isPermissionGiven) Log.i(TAG, "permission granted")
        else Log.i(TAG, "permission denied")

        val intent = Intent(this, MainService::class.java)
        intent.setAction(MainService.ACTION_HANDLE_NOTIFICATION_RESULT)
        intent.putExtra(
            MainService.EXTRA_ACCESS_KEY, PreferenceManager.getDefaultSharedPreferences(
                this
            ).getString(
                Constants.PREFS_KEY_SETTINGS_ACCESS_KEY, Defaults(
                    this
                ).accessKey
            )
        )
        startService(intent)
        finish()
    }

    companion object {
        private const val TAG = "NotificationRequestActivity"
        private const val REQUEST_POST_NOTIFICATION = 45
        private const val PREFS_KEY_POST_NOTIFICATION_PERMISSION_ASKED_BEFORE =
            "post_notification_permission_asked_before"
    }
}
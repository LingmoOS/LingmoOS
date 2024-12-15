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

class WriteStorageRequestActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // if file transfer not wanted, bail out early without bothering the user
        if (!intent.getBooleanExtra(
                MainService.EXTRA_FILE_TRANSFER, Defaults(
                    this
                ).fileTransfer
            )
        ) {
            postResultAndFinish(false)
            return
        }

        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Log.i(TAG, "Has no permission! Ask!")
            val prefs = PreferenceManager.getDefaultSharedPreferences(this)

            /*
                As per as per https://stackoverflow.com/a/34612503/361413 shouldShowRequestPermissionRationale()
                returns false also if user was never asked, so keep track of that with a shared preference. Ouch.
             */
            if (!prefs.getBoolean(
                    PREFS_KEY_PERMISSION_ASKED_BEFORE,
                    false
                ) || shouldShowRequestPermissionRationale(
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
                )
            ) {
                AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setTitle(R.string.write_storage_title)
                    .setMessage(if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) R.string.write_storage_msg_android_11 else R.string.write_storage_msg)
                    .setPositiveButton(R.string.yes) { dialog: DialogInterface?, which: Int ->
                        requestPermissions(
                            arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE),
                            REQUEST_WRITE_STORAGE
                        )
                        val ed = prefs.edit()
                        ed.putBoolean(PREFS_KEY_PERMISSION_ASKED_BEFORE, true)
                        ed.apply()
                    }
                    .setNegativeButton(getString(R.string.no)) { dialog: DialogInterface?, which: Int ->
                        postResultAndFinish(
                            false
                        )
                    }
                    .show()
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
        if (requestCode == REQUEST_WRITE_STORAGE) {
            postResultAndFinish(grantResults.size > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
        }
    }

    private fun postResultAndFinish(isPermissionGiven: Boolean) {
        if (isPermissionGiven) Log.i(TAG, "permission granted")
        else Log.i(TAG, "permission denied")

        val intent = Intent(this, MainService::class.java)
        intent.setAction(MainService.ACTION_HANDLE_WRITE_STORAGE_RESULT)
        intent.putExtra(
            MainService.EXTRA_ACCESS_KEY, PreferenceManager.getDefaultSharedPreferences(
                this
            ).getString(
                Constants.PREFS_KEY_SETTINGS_ACCESS_KEY, Defaults(
                    this
                ).accessKey
            )
        )
        intent.putExtra(MainService.EXTRA_WRITE_STORAGE_RESULT, isPermissionGiven)
        startService(intent)
        finish()
    }

    companion object {
        private const val TAG = "WriteStorageRequestActivity"
        private const val REQUEST_WRITE_STORAGE = 44
        private const val PREFS_KEY_PERMISSION_ASKED_BEFORE =
            "write_storage_permission_asked_before"
    }
}
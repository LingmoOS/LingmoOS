// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

import android.app.AlarmManager
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.SystemClock
import android.util.Log
import androidx.preference.PreferenceManager
import com.lingmo.assistant.services.MainService

class OnBootReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, arg1: Intent) {
        if (Intent.ACTION_BOOT_COMPLETED == arg1.action) {
            val prefs = PreferenceManager.getDefaultSharedPreferences(context)
            val defaults = Defaults(context)
            if (prefs.getBoolean(
                    Constants.PREFS_KEY_SETTINGS_START_ON_BOOT,
                    defaults.startOnBoot
                )
            ) {
                // autostart needs InputService on Android 10 and newer, both for the activity starts from MainService
                // (could be reworked) but most importantly for fallback screen capture

                if (Build.VERSION.SDK_INT >= 30 && !InputService.isConnected) {
                    Log.w(TAG, "onReceive: configured to start, but on Android 10+ and InputService not set up, bailing out")
                    return
                }

                val intent = Intent(context, MainService::class.java)
                intent.setAction(MainService.ACTION_START)
                intent.putExtra(
                    MainService.EXTRA_PORT,
                    prefs.getInt(Constants.PREFS_KEY_SETTINGS_PORT, defaults.port)
                )
                intent.putExtra(
                    MainService.EXTRA_PASSWORD,
                    prefs.getString(Constants.PREFS_KEY_SETTINGS_PASSWORD, defaults.password)
                )
                intent.putExtra(
                    MainService.EXTRA_FILE_TRANSFER,
                    prefs.getBoolean(
                        Constants.PREFS_KEY_SETTINGS_FILE_TRANSFER,
                        defaults.fileTransfer
                    )
                )
                intent.putExtra(
                    MainService.EXTRA_VIEW_ONLY,
                    prefs.getBoolean(Constants.PREFS_KEY_SETTINGS_VIEW_ONLY, defaults.viewOnly)
                )
                intent.putExtra(
                    MainService.EXTRA_SCALING,
                    prefs.getFloat(Constants.PREFS_KEY_SETTINGS_SCALING, defaults.scaling)
                )
                intent.putExtra(
                    MainService.EXTRA_ACCESS_KEY,
                    prefs.getString(Constants.PREFS_KEY_SETTINGS_ACCESS_KEY, defaults.accessKey)
                )
                intent.putExtra(
                    MainService.EXTRA_FALLBACK_SCREEN_CAPTURE,
                    true
                ) // want this on autostart

                val delayMillis = 1000L * prefs.getInt(
                    Constants.PREFS_KEY_SETTINGS_START_ON_BOOT_DELAY,
                    defaults.startOnBootDelay
                )
                if (delayMillis > 0) {
                    Log.i(TAG, "onReceive: configured to start delayed by " + delayMillis / 1000 + "s")
                    val alarmManager =
                        context.getSystemService(Context.ALARM_SERVICE) as AlarmManager
                    val pendingIntent = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                        PendingIntent.getForegroundService(
                            context.applicationContext,
                            0,
                            intent,
                            PendingIntent.FLAG_IMMUTABLE
                        )
                    } else {
                        PendingIntent.getService(
                            context.applicationContext,
                            0,
                            intent,
                            PendingIntent.FLAG_IMMUTABLE
                        )
                    }
                    alarmManager.setAndAllowWhileIdle(
                        AlarmManager.ELAPSED_REALTIME_WAKEUP,
                        SystemClock.elapsedRealtime() + delayMillis,
                        pendingIntent
                    )
                } else {
                    Log.i(TAG, "onReceive: configured to start immediately")
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                        context.applicationContext.startForegroundService(intent)
                    } else {
                        context.applicationContext.startService(intent)
                    }
                }
            } else {
                Log.i(TAG, "onReceive: configured NOT to start")
            }
        }
    }

    companion object {
        private const val TAG = "OnBootReceiver"
    }
}

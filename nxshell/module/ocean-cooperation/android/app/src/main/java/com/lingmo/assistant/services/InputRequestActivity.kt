// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.services

import android.app.AlertDialog
import android.content.ActivityNotFoundException
import android.content.DialogInterface
import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.provider.Settings
import android.util.Log
import android.view.Gravity
import android.view.View
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.PreferenceManager
import com.lingmo.assistant.R


class InputRequestActivity : AppCompatActivity() {
    private var mDoNotStartMainServiceOnFinish = false


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (intent.getBooleanExtra(EXTRA_DO_NOT_START_MAIN_SERVICE_ON_FINISH, false)) {
            mDoNotStartMainServiceOnFinish = true
        }

        /*
            Get whether input and/or start-on-boot are requested; directly or from prefs as fallback.
         */
        val startOnBootRequested =
            Build.VERSION.SDK_INT >= 30 && PreferenceManager.getDefaultSharedPreferences(
                this
            ).getBoolean(
                Constants.PREFS_KEY_SETTINGS_START_ON_BOOT, Defaults(
                    this
                ).startOnBoot
            )
        val inputRequested = if (intent.hasExtra(MainService.EXTRA_VIEW_ONLY)
        ) !intent.getBooleanExtra(
            MainService.EXTRA_VIEW_ONLY, Defaults(
                this
            ).viewOnly
        )
        else !PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
            Constants.PREFS_KEY_SETTINGS_VIEW_ONLY, Defaults(
                this
            ).viewOnly
        )

        Log.d(TAG, "onCreate: input requested: $inputRequested start on boot requested: $startOnBootRequested")


        /*
            If neither input nor start-on-boot are requested, bail out early without bothering the user.
         */
        if (!inputRequested && !startOnBootRequested) {
            postResultAndFinish(false)
            return
        }

        /*
            Then, decide on message to be shown.
         */
        val msg = if (inputRequested && startOnBootRequested) {
            // input and boot requested
            R.string.input_a11y_msg_input_and_boot
        } else if (inputRequested) {
            // input requested
            R.string.input_a11y_msg_input
        } else {
            // boot requested
            R.string.input_a11y_msg_boot
        }

        if (!InputService.isConnected) {
            // 创建一个 TextView 来显示消息
            val messageView = TextView(this)
            messageView.setText(msg)
            messageView.textAlignment = View.TEXT_ALIGNMENT_VIEW_START // 设置文本对齐方式为两端对齐

            messageView.gravity = Gravity.LEFT // 确保左对齐

            messageView.layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            messageView.setPadding(50, 50, 50, 50) // 设置内边距


            AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.input_a11y_title)
                .setView(messageView)
                .setPositiveButton(R.string.yes) { dialog: DialogInterface?, which: Int ->
                    val intent = Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS)
                    // highlight entry on some devices, see https://stackoverflow.com/a/63214655/361413
                    val EXTRA_FRAGMENT_ARG_KEY = ":settings:fragment_args_key"
                    val EXTRA_SHOW_FRAGMENT_ARGUMENTS = ":settings:show_fragment_args"
                    val bundle = Bundle()
                    val showArgs = packageName + "/" + InputService::class.java.name
                    bundle.putString(EXTRA_FRAGMENT_ARG_KEY, showArgs)
                    intent.putExtra(EXTRA_FRAGMENT_ARG_KEY, showArgs)
                    intent.putExtra(EXTRA_SHOW_FRAGMENT_ARGUMENTS, bundle)
                    if (intent.resolveActivity(packageManager) != null && !intent.resolveActivity(
                            packageManager
                        ).toString().contains("Stub")
                    ) startActivityForResult(intent, REQUEST_INPUT)
                    else AlertDialog.Builder(this@InputRequestActivity)
                        .setMessage(R.string.input_a11y_act_not_found_msg)
                        .setPositiveButton(android.R.string.ok) { dialogInterface: DialogInterface?, i: Int ->
                            val generalSettingsIntent = Intent(
                                Settings.ACTION_SETTINGS
                            )
                            try {
                                startActivityForResult(generalSettingsIntent, REQUEST_INPUT)
                            } catch (ignored: ActivityNotFoundException) {
                                // This should not happen, but there were crashes reported from flaky devices
                                // so in this case do nothing instead of crashing.
                            }
                        }
                        .show()
                }
                .setNegativeButton(getString(R.string.no)) { dialog: DialogInterface?, which: Int ->
                    postResultAndFinish(
                        false
                    )
                }
                .show()
        } else {
            postResultAndFinish(true)
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == REQUEST_INPUT) {
            Log.d(TAG, "onActivityResult")
            postResultAndFinish(InputService.isConnected)
        }
    }

    private fun postResultAndFinish(isA11yEnabled: Boolean) {
        if (isA11yEnabled) {
            Log.i(TAG, "a11y enabled")
        } else {
            Log.i(TAG, "a11y disabled")
            //            return;
        }

        if (!mDoNotStartMainServiceOnFinish) {
            val intent = Intent(this, MainService::class.java)
            intent.setAction(MainService.ACTION_HANDLE_INPUT_RESULT)
            intent.putExtra(MainService.EXTRA_INPUT_RESULT, isA11yEnabled)
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
        }
        finish()
    }

    companion object {
        private const val TAG = "InputRequestActivity"
        private const val REQUEST_INPUT = 43
        const val EXTRA_DO_NOT_START_MAIN_SERVICE_ON_FINISH: String =
            "do_not_start_main_service_on_finish"
    }
}
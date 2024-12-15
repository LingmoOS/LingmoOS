// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.ui.fragment

import android.util.Log
import android.view.View
import android.widget.ImageView
import android.widget.TextView
import androidx.appcompat.widget.AppCompatButton
import androidx.lifecycle.Observer
import androidx.lifecycle.ViewModelProvider
import com.lingmo.assistant.R
import com.lingmo.assistant.aop.SingleClick
import com.lingmo.assistant.app.TitleBarFragment
import com.lingmo.assistant.model.SharedViewModel
import com.lingmo.assistant.ui.activity.HomeActivity
import com.lingmo.assistant.utils.Utils
import com.lingmo.cooperation.JniCooperation

class HomeFragment : TitleBarFragment<HomeActivity>() {

    companion object {
        private const val TAG = "HomeFragment"
        fun newInstance(): HomeFragment {
            return HomeFragment()
        }
    }

    private lateinit var viewModel: SharedViewModel

    private val statusTextView: TextView? by lazy { findViewById(R.id.tv_status_text) }
    private val nameTextView: TextView? by lazy { findViewById(R.id.tv_device_name) }
    private val ipTextView: TextView? by lazy { findViewById(R.id.tv_device_ip) }
    private val deviceInfoTextView: TextView? by lazy { findViewById(R.id.tv_device_info) }
    private val statusImageView: ImageView? by lazy { findViewById(R.id.iv_status_icon) }
    private val startButton: AppCompatButton? by lazy { findViewById(R.id.btn_cast_start) }
    private val stopButton: AppCompatButton? by lazy { findViewById(R.id.btn_cast_stop) }

    override fun getLayoutId(): Int {
        return R.layout.home_fragment
    }

    override fun initView() {
        setOnClickListener(startButton)
        setOnClickListener(stopButton)

        activity?.let {
            viewModel = ViewModelProvider(it).get(SharedViewModel::class.java)
        }

        viewModel.selfIp().observe(viewLifecycleOwner, Observer {
            if (!it.isBlank()) {
                Log.d(HomeFragment.TAG, "the selfip update: $it")
                val deviceName = Utils.getDeviceName(requireContext())
                val ipAddress = it
                deviceInfoTextView?.text = "$deviceName | IP：${ipAddress ?: ""}"
            }
        })

        viewModel.name().observe(viewLifecycleOwner, Observer {
            if (!it.isBlank()) {
                Log.d(TAG, "the name update: $it")
                nameTextView?.text = it
            }
        })
        viewModel.ip().observe(viewLifecycleOwner, Observer {
            if (!it.isBlank()) {
                Log.d(TAG, "the address update: $it")
                ipTextView?.text = "IP：${it}"
            }
        })
        viewModel.isScreenMirroring().observe(viewLifecycleOwner, Observer {
            val screenCastingMessage = getString(R.string.cooperation_activity_screen_casting)
            val endScreenCastingMessage = getString(R.string.cooperation_activity_end_screen)
            val cooperationMessage = getString(R.string.cooperation_activity_cooperation)
            val screenCast = getString(R.string.cooperation_activity_screencast)
            Log.d(TAG, "isScreenMirroring: $it")
            if (it) {
                startButton?.text = endScreenCastingMessage
                statusTextView?.text = screenCastingMessage
                statusImageView?.setImageResource(R.mipmap.screen_cast_ic)
            }else{
                startButton?.text = screenCast
                statusTextView?.text = cooperationMessage
                statusImageView?.setImageResource(R.mipmap.cooperation_ic)

            }
        })
    }

    override fun initData() {
        val deviceName = Utils.getDeviceName(requireContext())
        val ipAddress = Utils.getIPAddress(requireContext())
        deviceInfoTextView?.text = "$deviceName | IP：${ipAddress ?: ""}"
    }

    override fun isStatusBarEnabled(): Boolean {
        // 使用沉浸式状态栏
        return !super.isStatusBarEnabled()
    }

    @SingleClick
    override fun onClick(view: View) {
        if (view === startButton) {
            val screenCast = getString(R.string.cooperation_activity_screencast)

            if (startButton?.text == screenCast) {
                viewModel.doAction(JniCooperation.ACTION_PROJECTION_START)
            } else {
                // TODO: show dialog?
                viewModel.doAction(JniCooperation.ACTION_PROJECTION_STOP)
            }

        } else if (view === stopButton) {
            HomeActivity.start(getAttachActivity()!!, FirstFragment::class.java)
            viewModel.doAction(JniCooperation.ACTION_PROJECTION_DISCONNECT)

            viewModel.setScreenMirroring(false)
        }
    }
}
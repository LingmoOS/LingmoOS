// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.ui.fragment

import android.content.Intent
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.AppCompatButton
import androidx.lifecycle.ViewModelProvider
import com.lingmo.assistant.R
import com.lingmo.assistant.aop.SingleClick
import com.lingmo.assistant.app.TitleBarFragment
import com.lingmo.assistant.model.SharedViewModel
import com.lingmo.assistant.ui.activity.HomeActivity
import com.lingmo.assistant.ui.scan.ScanActivity
import com.lingmo.assistant.utils.Utils
import androidx.lifecycle.Observer


class FirstFragment : TitleBarFragment<HomeActivity>() {

    companion object {
        private const val TAG = "ScanFragment"

        fun newInstance(): FirstFragment {
            return FirstFragment()
        }
    }

    private lateinit var viewModel: SharedViewModel
    // 初始化视图
    private val button: AppCompatButton? by lazy { findViewById(R.id.button) }
    private val tTextView: TextView? by lazy { findViewById(R.id.title) }
    private val deviceInfoTextView: TextView? by lazy { findViewById(R.id.device_info) }

    override fun getLayoutId(): Int {
        return R.layout.first_fragment
    }

    override fun initView() {
        setOnClickListener(button)

        activity?.let {
            viewModel = ViewModelProvider(it).get(SharedViewModel::class.java)
        }

        viewModel.selfIp().observe(viewLifecycleOwner, Observer {
            if (!it.isBlank()) {
                Log.d(FirstFragment.TAG, "the selfip update: $it")
                val deviceName = Utils.getDeviceName(requireContext())
                val ipAddress = it
                deviceInfoTextView?.text = "$deviceName | IP：${ipAddress ?: ""}"
            }
        })
    }

    override fun initData() {
    }

    private val barcodeLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == AppCompatActivity.RESULT_OK) {
            val data = result.data
            data?.let {
                val code = it.getStringExtra("SCAN_RESULT")
                val resultIntent = Intent()
                resultIntent.putExtra("SCAN_RESULT", code)
                requireActivity().setResult(AppCompatActivity.RESULT_OK, resultIntent)
            }
        }
    }

    private fun scanForDevices() {
        val intent = Intent(requireActivity(), ScanActivity::class.java)
        barcodeLauncher.launch(intent)
    }

    @SingleClick
    override fun onClick(view: View) {
        if (view === button) {
            scanForDevices()
        }
    }

    override fun isStatusBarEnabled(): Boolean {
        // 使用沉浸式状态栏
        return !super.isStatusBarEnabled()
    }
}
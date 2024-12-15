// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.ui.activity

import android.annotation.SuppressLint
import android.app.Activity
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.net.ConnectivityManager
import android.net.Network
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.Settings
import android.util.Base64
import android.util.Log
import android.view.Display
import android.view.View
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import androidx.lifecycle.Observer
import androidx.lifecycle.ViewModelProvider
import androidx.preference.PreferenceManager
import androidx.viewpager.widget.ViewPager
import com.lingmo.assistant.R
import com.lingmo.assistant.app.AppActivity
import com.lingmo.assistant.app.AppFragment
import com.lingmo.assistant.manager.ActivityManager
import com.lingmo.assistant.model.SharedViewModel
import com.lingmo.assistant.other.DoubleClickHelper
import com.lingmo.assistant.ui.adapter.NavigationAdapter
import com.lingmo.assistant.ui.fragment.FirstFragment
import com.lingmo.assistant.ui.fragment.HomeFragment
import com.lingmo.assistant.ui.fragment.MessageFragment
import com.lingmo.assistant.ui.fragment.MineFragment
import com.lingmo.cooperation.CooperationListener
import com.lingmo.cooperation.JniCooperation
import com.gyf.immersionbar.ImmersionBar
import com.hjq.base.FragmentPagerAdapter
import com.hjq.permissions.Permission
import com.lingmo.assistant.utils.Utils
import com.lingmo.assistant.services.Constants
import com.lingmo.assistant.services.Defaults
import com.lingmo.assistant.services.InputService
import com.lingmo.assistant.services.MainService
import com.lingmo.assistant.services.MediaProjectionService
import org.json.JSONObject

class HomeActivity : AppActivity(), NavigationAdapter.OnNavigationListener {

    companion object {
        private const val TAG = "HomeActivity"
        private const val INTENT_KEY_IN_FRAGMENT_INDEX: String = "fragmentIndex"
        private const val INTENT_KEY_IN_FRAGMENT_CLASS: String = "fragmentClass"

        @JvmOverloads
        fun start(context: Context, fragmentClass: Class<out AppFragment<*>?>? = FirstFragment::class.java) {
            val intent = Intent(context, HomeActivity::class.java)
            intent.putExtra(INTENT_KEY_IN_FRAGMENT_CLASS, fragmentClass)
            if (context !is Activity) {
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(intent)
        }
    }

    private lateinit var connectivityManager: ConnectivityManager
    private lateinit var networkCallback: ConnectivityManager.NetworkCallback

    private lateinit var viewModel: SharedViewModel
    private var mCooperation: JniCooperation? = null

    private var mIsMainServiceRunning = false
    private var mMainServiceBroadcastReceiver: BroadcastReceiver? = null
    private var mDefaults: Defaults? = null

    private val viewPager: ViewPager? by lazy { findViewById(R.id.vp_home_pager) }
    private var navigationAdapter: NavigationAdapter? = null
    private var pagerAdapter: FragmentPagerAdapter<AppFragment<*>>? = null

    override fun getLayoutId(): Int {
        return R.layout.home_activity
    }

    fun scanConnect() {
        val metrics = Utils.getDisplayMetrics(this, Display.DEFAULT_DISPLAY)
        mCooperation?.scanConnect(metrics.widthPixels, metrics.heightPixels)
    }
    override fun initView() {
        navigationAdapter = NavigationAdapter(this).apply {
            addItem(NavigationAdapter.MenuItem(getString(R.string.home_nav_found),
                ContextCompat.getDrawable(this@HomeActivity, R.drawable.home_found_selector)))
            addItem(NavigationAdapter.MenuItem(getString(R.string.home_nav_index),
                ContextCompat.getDrawable(this@HomeActivity, R.drawable.home_home_selector)))
            addItem(NavigationAdapter.MenuItem(getString(R.string.home_nav_message),
                ContextCompat.getDrawable(this@HomeActivity, R.drawable.home_message_selector)))
            addItem(NavigationAdapter.MenuItem(getString(R.string.home_nav_me),
                ContextCompat.getDrawable(this@HomeActivity, R.drawable.home_me_selector)))
            setOnNavigationListener(this@HomeActivity)
        }

        mCooperation = JniCooperation()
        val hosts = MainService.getIPv4s()

        viewModel = ViewModelProvider(this).get(SharedViewModel::class.java)
        if (hosts.isNotEmpty()) {
            val host = hosts[0]
            mCooperation?.initNative(host)
            viewModel.updateSelfIp(host)
        } else {
            mCooperation?.initNative("")
            viewModel.updateSelfIp(getString(R.string.scan_activity_no_network))
        }

        mCooperation?.registerListener(object : CooperationListener {
            override fun onConnectChanged(result: Int, reason: String) {
                Log.d(TAG, "Connection changed: result $result, reason: $reason")
                when (result) {
                    JniCooperation.CONNECT_STATUS_FALSE -> {
                        toast(R.string.scan_activity_disconnect)
                        start(this@HomeActivity, FirstFragment::class.java)
                        initScreenMirroring()
                    }
                }
            }

            @SuppressLint("LogNotTimber")
            override fun onAsyncRpcResult(type: Int, response: String) {
                Log.d(TAG, "Async RPC result: type $type, response: $response")
                when (type) {
                    JniCooperation.RPC_APPLY_LOGIN -> {
                        //type 1000, response: {"auth":"thatsgood","name":"10.8.11.52"}
                        if (response.isNullOrEmpty()) {
                            Log.e(TAG, "FAIL to connect...")
                            // TODO: show connection failed diaglog.
                            return
                        }
                        val jsonObject = JSONObject(response)
                        val auth = jsonObject.getString("auth")
                        val name = jsonObject.getString("name")
                        Log.d(TAG, "Login success: name $name, auth: $auth")
                        scanConnect();
                    }
                    JniCooperation.RPC_APPLY_SCAN_CONNECT -> {
                        if (response.isNullOrEmpty()) {
                            Log.e(TAG, "FAIL to connect...")
                            // TODO: show connection failed diaglog.
                            return
                        }
                        val jsonObject = JSONObject(response)
                        val nick = jsonObject.getString("nick")
                        val pcInfo = JSONObject(nick)
                        val ip = pcInfo.getString("IPAddress")
                        val name = pcInfo.getString("DeviceName")

                        Log.d(TAG, "SCAN_CONNECT success: name $name, auth: $ip")
                        this@HomeActivity.viewModel.updateInfo(name, ip)
                        start(this@HomeActivity, HomeFragment::class.java)
                    }
                    JniCooperation.RPC_APPLY_PROJECTION -> {
                        // start the vnc service after send the projection rpc success.
                        //switchVncServer()
                    }
                    JniCooperation.RPC_APPLY_PROJECTION_RESULT -> {
                    }
                    JniCooperation.RPC_APPLY_PROJECTION_STOP -> {
                        switchVncServer()
                    }
                }
            }
        })

        mMainServiceBroadcastReceiver = object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                if (MainService.ACTION_START == intent.action) {
                    if (intent.getBooleanExtra(MainService.EXTRA_REQUEST_SUCCESS, false)) {
                        // was a successful START requested by anyone (but sent by MainService, as the receiver is not exported!)
                        Log.d(TAG, "got MainService started success event")
                        onServerStarted()
                    } else {
                        // was a failed START requested by anyone (but sent by MainService, as the receiver is not exported!)
                        Log.d(TAG, "got MainService started fail event")
                        onServerFailed()
                    }
                }

                if (MainService.ACTION_STOP == intent.action && (intent.getBooleanExtra(
                        MainService.EXTRA_REQUEST_SUCCESS,
                        true
                    ))
                ) {
                    // was a successful STOP requested by anyone (but sent by MainService, as the receiver is not exported!)
                    // or a STOP without any extras
                    Log.d(TAG, "got MainService stopped event")
                    onServerStopped()
                }
            }
        }
        val filter = IntentFilter()
        filter.addAction(MainService.ACTION_START)
        filter.addAction(MainService.ACTION_STOP)
        filter.addAction(MainService.ACTION_CONNECT_REVERSE)
        filter.addAction(MainService.ACTION_CONNECT_REPEATER)
        // register the receiver as NOT_EXPORTED so it only receives broadcasts sent by MainService,
        // not a malicious fake broadcaster like
        // `adb shell am broadcast -a com.lingmo.assistant.ACTION_STOP --ez com.lingmo.assistant.EXTRA_REQUEST_SUCCESS true`
        // for instance
        // androidx 1.8.0 以上高版本，SDK34
        // ContextCompat.registerReceiver(this, mMainServiceBroadcastReceiver, filter, ContextCompat.RECEIVER_NOT_EXPORTED)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(mMainServiceBroadcastReceiver, filter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(mMainServiceBroadcastReceiver, filter);
        }

        // setup UI initial state
        if (MainService.isServerActive()) {
            Log.d(TAG, "Found server to be started")
            onServerStarted()
        } else {
            Log.d(TAG, "Found server to be stopped")
            onServerStopped()
        }
    }

    override fun initData() {
        pagerAdapter = FragmentPagerAdapter<AppFragment<*>>(this).apply {
            addFragment(FirstFragment.newInstance())
            addFragment(HomeFragment.newInstance())
            addFragment(MessageFragment.newInstance())
            addFragment(MineFragment.newInstance())
            viewPager?.adapter = this
        }
        onNewIntent(intent)

        connectivityManager = getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        // 创建并注册 NetworkCallback
        networkCallback = object : ConnectivityManager.NetworkCallback() {
            override fun onAvailable(network: Network) {
                Log.d(TAG, "执行网络连接时的操作")
                // 网络连接可用时调用
                runOnUiThread {
                    // 执行网络连接时的操作
                    val hosts = MainService.getIPv4s()
                    val host = hosts[0]
                    mCooperation?.initNative(host)
                    viewModel.updateSelfIp(host)
                }
            }

            override fun onLost(network: Network) {
                // 网络连接丢失时调用
                Log.d(TAG, "网络连接丢失时调用")
                runOnUiThread {
                    // 执行网络断开时的操作
                    viewModel.updateSelfIp(getString(R.string.scan_activity_no_network))
                }
            }
        }

        // 注册回调
        connectivityManager.registerDefaultNetworkCallback(networkCallback)

        val deviceName = Utils.getDeviceName(this)
        deviceName?.let { mCooperation?.setDeviceName(it) }

        viewModel = ViewModelProvider(this).get(SharedViewModel::class.java)
        viewModel.action().observe(this, Observer {
            Log.d(TAG, "will do the action: $it")
            when (it) {
                JniCooperation.ACTION_PROJECTION_START -> {
                    switchVncServer()
                }

                JniCooperation.ACTION_PROJECTION_STOP -> {
                    deviceName?.let { it -> mCooperation?.stopProjection(it) }
                }

                JniCooperation.ACTION_PROJECTION_DISCONNECT -> {
                    mCooperation?.disconnectRemote()
                }
            }
        })
    }

    override fun onNewIntent(intent: Intent?) {
        super.onNewIntent(intent)
        pagerAdapter?.let {
            switchFragment(it.getFragmentIndex(getSerializable(INTENT_KEY_IN_FRAGMENT_CLASS)))
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        viewPager?.let {
            // 保存当前 Fragment 索引位置
            outState.putInt(INTENT_KEY_IN_FRAGMENT_INDEX, it.currentItem)
        }
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
        super.onRestoreInstanceState(savedInstanceState)
        // 恢复当前 Fragment 索引位置
        switchFragment(savedInstanceState.getInt(INTENT_KEY_IN_FRAGMENT_INDEX))
    }

    private fun switchFragment(fragmentIndex: Int) {
        if (fragmentIndex == -1) {
            return
        }
        when (fragmentIndex) {
            0, 1, 2, 3 -> {
                viewPager?.currentItem = fragmentIndex
                navigationAdapter?.setSelectedPosition(fragmentIndex)
            }
        }
    }

    /**
     * [NavigationAdapter.OnNavigationListener]
     */
    override fun onNavigationItemSelected(position: Int): Boolean {
        return when (position) {
            0, 1, 2, 3 -> {
                viewPager?.currentItem = position
                true
            }
            else -> false
        }
    }

    override fun createStatusBarConfig(): ImmersionBar {
        return super.createStatusBarConfig() // 指定导航栏背景颜色
            .navigationBarColor(R.color.white)
    }

    override fun onBackPressed() {
        if (!DoubleClickHelper.isOnDoubleClick()) {
            toast(R.string.home_exit_hint)
            return
        }

        // 移动到上一个任务栈，避免侧滑引起的不良反应
        moveTaskToBack(false)
        postDelayed({
            // 进行内存优化，销毁掉所有的界面
            ActivityManager.getInstance().finishAllActivities()
        }, 300)
    }

    @SuppressLint("SetTextI18n")
    override fun onResume() {
        super.onResume()
    }

    override fun onDestroy() {
        super.onDestroy()
        viewPager?.adapter = null

        navigationAdapter?.setOnNavigationListener(null)
        unregisterReceiver(mMainServiceBroadcastReceiver)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode == RESULT_OK) {
            val resultCodeFromScan = data?.getStringExtra("SCAN_RESULT")
            Log.d(TAG, "扫描结果传回 HomeActivity: $resultCodeFromScan")

            initScreenMirroring()

            resultCodeFromScan?.let {
                try {
                    var mark = it.substringAfter("mark=")
                    if (mark.isNullOrEmpty()) {
                        mark = it // without mark=, back to old.
                    }
                    val decodedBytes = Base64.decode(mark, Base64.DEFAULT)
                    // 处理成功解码的字节
                    val decodedString = String(decodedBytes)

                    // 按照"&"分割字符串
                    val parameters = decodedString.split("&")
                    if (parameters.size >= 3) {
                        // 创建一个存储解析结果的 Map
                        val resultMap = mutableMapOf<String, String>()

                        // 遍历参数并进行分割
                        for (parameter in parameters) {
                            val keyValue = parameter.split("=")
                            if (keyValue.size == 2) {
                                resultMap[keyValue[0]] = keyValue[1]
                            } else {
                                Log.e(TAG, "解析出错，参数格式不正确：$parameter")
                            }
                        }

                        // 提取出 ip、port 和 pin
                        val ip = resultMap["host"]
                        val portString = resultMap["port"]
                        val pin = resultMap["pin"]
                        val pv = resultMap["pv"] // protocol version

                        // 将 port 转换为 Int，如果失败默认为 0
                        val port = portString?.toIntOrNull() ?: 0

                        if (ip != null && pin != null) {
                            // 调用连接方法
                            val ret =  connectCooperation(ip, port, pin)
                            if (ret == -1)
                                Toast.makeText(this, R.string.scan_activity_connect_fail, Toast.LENGTH_LONG).show()
                            else
                                Log.e(TAG, "连接成功：host=$ip, port=$portString, pin=$pin")
                        } else {
                            Log.e(TAG, "解析出错，参数不完整：host=$ip, port=$portString, pin=$pin")
                        }
                    } else {
                        Toast.makeText(this, R.string.scan_activity_error, Toast.LENGTH_LONG).show()
                        Log.e(TAG, "解析出错，参数数量不匹配：$decodedString")
                    }
                } catch (e: IllegalArgumentException) {
                    // 捕获并处理非法参数异常
                    Toast.makeText(this, R.string.scan_activity_error, Toast.LENGTH_LONG).show()
                    e.printStackTrace()
                }
            }

        }
    }

    fun connectCooperation(ip: String, port: Int, pin: String): Int{
        val ret =  mCooperation?.connectRemote(ip, port, pin) ?: return -1
        return ret;
    }

    fun initScreenMirroring() {
        this@HomeActivity.viewModel?.setScreenMirroring(false)
        if (mIsMainServiceRunning)
            switchVncServer()
    }

    fun switchVncServer() {
        val prefs = PreferenceManager.getDefaultSharedPreferences(this)
        mDefaults = Defaults(this)

        val intent = Intent(this@HomeActivity, MainService::class.java)
        intent.putExtra(
            MainService.EXTRA_PORT,
            prefs.getInt(Constants.PREFS_KEY_SETTINGS_PORT, mDefaults!!.port)
        )
        intent.putExtra(
            MainService.EXTRA_PASSWORD,
            prefs.getString(Constants.PREFS_KEY_SETTINGS_PASSWORD, mDefaults!!.password)
        )
        intent.putExtra(
            MainService.EXTRA_FILE_TRANSFER,
            prefs.getBoolean(
                Constants.PREFS_KEY_SETTINGS_FILE_TRANSFER,
                mDefaults!!.fileTransfer
            )
        )
        intent.putExtra(
            MainService.EXTRA_VIEW_ONLY,
            prefs.getBoolean(Constants.PREFS_KEY_SETTINGS_VIEW_ONLY, mDefaults!!.viewOnly)
        )
        intent.putExtra(
            MainService.EXTRA_SHOW_POINTERS,
            false // disable show pointer
//            prefs.getBoolean(
//                Constants.PREFS_KEY_SETTINGS_SHOW_POINTERS,
//                mDefaults!!.showPointers
//            )
        )
        intent.putExtra(
            MainService.EXTRA_SCALING,
            prefs.getFloat(Constants.PREFS_KEY_SETTINGS_SCALING, mDefaults!!.scaling)
        )
        intent.putExtra(
            MainService.EXTRA_ACCESS_KEY,
            prefs.getString(Constants.PREFS_KEY_SETTINGS_ACCESS_KEY, mDefaults!!.accessKey)
        )
        if (mIsMainServiceRunning) {
            intent.setAction(MainService.ACTION_STOP)
        } else {
            intent.setAction(MainService.ACTION_START)
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(intent)
        } else {
            startService(intent)
        }
    }

    private fun onServerFailed() {
    }

    private fun onServerStarted() {
        viewModel = ViewModelProvider(this).get(SharedViewModel::class.java)
        this@HomeActivity.viewModel.setScreenMirroring(true)
        mIsMainServiceRunning = true

        val deviceName = Utils.getDeviceName(this)
        deviceName?.let { it -> mCooperation?.sendProjection(it, 5900) }
    }

    private fun onServerStopped() {
        viewModel = ViewModelProvider(this).get(SharedViewModel::class.java)
        this@HomeActivity.viewModel.setScreenMirroring(false)
        mIsMainServiceRunning = false
    }
}
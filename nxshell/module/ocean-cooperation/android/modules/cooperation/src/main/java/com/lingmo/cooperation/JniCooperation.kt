// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.cooperation

import android.util.Log

class JniCooperation {
    private val TAG = "JniCooperation"

    private var listener: CooperationListener? = null


    /**
     * Native methods that are implemented by the 'cooperation' native library,
     * which is packaged with this application.
     */
    external fun nativeVersion(): String
    external fun initNative(ip: String)
    external fun setStorageFolder(rootPath: String, dirName: String)
    external fun setDeviceName(nickName: String)
    external fun connectRemote(ip: String, port: Int, pin: String): Int
    external fun disconnectRemote()
    external fun scanConnect(width: Int, height: Int)
    external fun sendProjection(niceName: String, vncPort: Int)
    external fun stopProjection(niceName: String)
    external fun getStatus(): Int

    companion object {
        // RPC 请求的类型，两端一致，不可随意变化
        const val RPC_APPLY_LOGIN = 1000 // 连接登录请求
        const val RPC_APPLY_SCAN_CONNECT = 200 // 扫码登录
        const val RPC_APPLY_PROJECTION = 201   // 投屏申请
        const val RPC_APPLY_PROJECTION_RESULT = 202   // 投屏申请的结果
        const val RPC_APPLY_PROJECTION_STOP = 203   // 收到停止投屏事件
        const val RPC_APPLY_PROJECTION_CANCELED = 204   // 投屏申请被取消

        // android 自定义动作
        const val ACTION_PROJECTION_START = 51
        const val ACTION_PROJECTION_STOP = 52
        const val ACTION_PROJECTION_DISCONNECT = 53

        // 连接状态
        const val CONNECT_STATUS_SUCCESS = 2
        const val CONNECT_STATUS_FALSE = -1

        // Used to load the 'cooperation' library on application startup.
        init {
            System.loadLibrary("cooperation")
        }
    }

    fun registerListener(listener: CooperationListener) {
        this.listener = listener
    }


    fun onConnectChanged(result: Int, reason: String) {
        Log.d(TAG, "onConnectChanged: result $result reason:$reason")

        try {
            listener?.onConnectChanged(result, reason)
        } catch (e: Exception) {
            // instance probably null
            Log.e(TAG, "onClientConnected: error: $e")
        }
    }


    fun onAsyncRpcResult(type: Int, response: String) {
        // Log.d(TAG, "onAsyncRpcResult: type $type response: $response")

        try {
            listener?.onAsyncRpcResult(type, response)
        } catch (e: Exception) {
            // instance probably null
            Log.e(TAG, "onAsyncRpcResult: error: $e")
        }
    }
}
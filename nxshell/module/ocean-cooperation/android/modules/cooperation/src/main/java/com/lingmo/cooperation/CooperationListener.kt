// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.cooperation

interface CooperationListener {
    fun onConnectChanged(result: Int, reason: String)
    fun onAsyncRpcResult(type: Int, response: String)
}
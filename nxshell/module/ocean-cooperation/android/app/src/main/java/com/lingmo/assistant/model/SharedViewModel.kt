// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package com.lingmo.assistant.model

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel

class SharedViewModel : ViewModel() {

    private val name: MutableLiveData<String> = MutableLiveData("")
    private val ip: MutableLiveData<String> = MutableLiveData("")
    private val selfIp: MutableLiveData<String> = MutableLiveData("")
    private val action: MutableLiveData<Int> = MutableLiveData(-1)
    private val isScreenMirroring: MutableLiveData<Boolean> = MutableLiveData(false)

    fun name(): LiveData<String> = name
    fun ip(): LiveData<String> = ip

    fun selfIp(): LiveData<String> = selfIp

    fun action(): LiveData<Int> = action

    fun isScreenMirroring(): LiveData<Boolean> = isScreenMirroring

    fun updateInfo(auth: String, addr: String) {
        name.postValue(auth)
        ip.postValue(addr)
        // set value failed by diff thread
//        name.value = auth
//        ip.value = addr
    }

    fun doAction(req: Int) {
        action.postValue(req)
    }

    fun updateSelfIp(addr: String) {
        selfIp.postValue(addr)
    }

    fun setScreenMirroring(req: Boolean) {
        isScreenMirroring.postValue(req)
    }

    init {
        // Setting default value
        name.value = ""
        ip.value = ""
        action.value = -1
        isScreenMirroring.value = false
    }
}
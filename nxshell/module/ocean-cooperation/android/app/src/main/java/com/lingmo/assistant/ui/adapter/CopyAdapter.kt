package com.lingmo.assistant.ui.adapter

import android.content.Context
import android.view.ViewGroup
import com.lingmo.assistant.R
import com.lingmo.assistant.app.AppAdapter

/**
 *    author : Android 轮子哥
 *    github : https://github.com/getActivity/AndroidProject-Kotlin
 *    time   : 2018/11/05
 *    desc   : 可进行拷贝的副本
 */
class CopyAdapter(context: Context) : AppAdapter<String?>(context) {

    override fun getItemCount(): Int {
        return 10
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        return ViewHolder()
    }

    inner class ViewHolder : AppViewHolder(R.layout.copy_item) {
        override fun onBindView(position: Int) {}
    }
}
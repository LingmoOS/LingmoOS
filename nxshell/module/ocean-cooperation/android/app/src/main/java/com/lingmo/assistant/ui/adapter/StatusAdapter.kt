package com.lingmo.assistant.ui.adapter

import android.content.*
import android.view.ViewGroup
import android.widget.TextView
import com.lingmo.assistant.R
import com.lingmo.assistant.app.AppAdapter

/**
 *    author : Android 轮子哥
 *    github : https://github.com/getActivity/AndroidProject-Kotlin
 *    time   : 2019/09/22
 *    desc   : 状态数据列表
 */
class StatusAdapter(context: Context) : AppAdapter<String?>(context) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        return ViewHolder()
    }

    inner class ViewHolder : AppViewHolder(R.layout.status_item) {

        private val textView: TextView? by lazy { findViewById(R.id.tv_status_text) }

        override fun onBindView(position: Int) {
            textView?.text = getItem(position)
        }
    }
}
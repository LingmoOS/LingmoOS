package com.lingmo.assistant.ui.scan

import android.content.Context
import android.util.AttributeSet
import android.util.Log
import android.widget.Toast
import com.ailiwean.core.Result
import com.ailiwean.core.view.ScanBarCallBack
import com.ailiwean.core.view.ScanLightViewCallBack
import com.ailiwean.core.view.style2.NBZxingView
import com.ailiwean.core.zxing.ScanTypeConfig
import com.google.android.cameraview.AspectRatio

class ScanView @JvmOverloads constructor(
    context: Context,
    attributeSet: AttributeSet? = null,
    def: Int = 0
) : NBZxingView(context, attributeSet, def) {
    private val TAG = "ScanView-qrcode"
    private var resultCallback: ScanActivity ? = null

    fun setResultCallback(cb: ScanActivity) {
        this.resultCallback = cb
    }

    override fun resultBack(content: Result) {
        //Toast.makeText(context, content.text, Toast.LENGTH_LONG).show()
        Log.d(TAG, "qrcode:" + content.text)
        resultCallback!!.setResult(content.text)
    }

    /***
     * 返回扫码类型
     * 1 ScanTypeConfig.HIGH_FREQUENCY 高频率格式(默认)
     * 2 ScanTypeConfig.ALL  所有格式
     * 3 ScanTypeConfig.ONLY_QR_CODE 仅QR_CODE格式
     * 4 ScanTypeConfig.TWO_DIMENSION 所有二维码格式
     * 5 ScanTypeConfig.ONE_DIMENSION 所有一维码格式
     */
    override fun configScanType(): ScanTypeConfig {
        return ScanTypeConfig.HIGH_FREQUENCY
    }

    fun toParse(string: String) {
        parseFile(string)
    }

    override fun provideAspectRatio(): AspectRatio {
        return AspectRatio.of(16, 9)
    }

    override fun resultBackFile(content: com.ailiwean.core.zxing.core.Result?) {
        if (content == null)
            Toast.makeText(context, "未扫描到内容", Toast.LENGTH_SHORT).show()
        else Toast.makeText(context, content.text, Toast.LENGTH_SHORT).show()
    }

    override fun isSupportAutoZoom(): Boolean {
        return false

    }

    override fun provideLightView(): ScanLightViewCallBack? {
        return null
    }

    override fun provideScanBarView(): ScanBarCallBack? {
        return null
    }
}
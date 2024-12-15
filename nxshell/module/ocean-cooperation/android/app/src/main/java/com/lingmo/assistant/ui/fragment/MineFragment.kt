package com.lingmo.assistant.ui.fragment

import android.content.ActivityNotFoundException
import android.content.Intent
import android.content.pm.ActivityInfo
import android.net.Uri
import android.view.*
import com.hjq.base.BaseDialog
import com.lingmo.assistant.R
import com.lingmo.assistant.aop.SingleClick
import com.lingmo.assistant.app.TitleBarFragment
import com.lingmo.assistant.ui.activity.*
import com.lingmo.assistant.ui.dialog.InputDialog
import com.lingmo.assistant.ui.dialog.MessageDialog
import com.tencent.bugly.crashreport.CrashReport
import java.util.*

/**
 *    author : Android 轮子哥
 *    github : https://github.com/getActivity/AndroidProject-Kotlin
 *    time   : 2018/10/18
 *    desc   : 我的 Fragment
 */
class MineFragment : TitleBarFragment<HomeActivity>() {

    companion object {

        fun newInstance(): MineFragment {
            return MineFragment()
        }
    }

    override fun getLayoutId(): Int {
        return R.layout.mine_fragment
    }

    override fun initView() {
        setOnClickListener(R.id.btn_mine_dialog, R.id.btn_mine_hint,
            R.id.btn_mine_about, R.id.btn_mine_guide, R.id.btn_mine_browser, R.id.btn_mine_crash, R.id.btn_mine_pay)
    }

    override fun initData() {}

    @SingleClick
    override fun onClick(view: View) {
        when (view.id) {
            R.id.btn_mine_hint -> {
                startActivity(StatusActivity::class.java)
            }
            R.id.btn_mine_about -> {
                startActivity(AboutActivity::class.java)
            }
            R.id.btn_mine_guide -> {
                startActivity(GuideActivity::class.java)
            }
            R.id.btn_mine_browser -> {

                InputDialog.Builder(getAttachActivity()!!)
                    .setTitle("跳转到网页")
                    .setContent("https://www.jianshu.com/u/f7bb67d86765")
                    .setHint("请输入网页地址")
                    .setConfirm(getString(R.string.common_confirm))
                    .setCancel(getString(R.string.common_cancel))
                    .setListener(object : InputDialog.OnListener {

                        override fun onConfirm(dialog: BaseDialog?, content: String) {
                            BrowserActivity.start(getAttachActivity()!!, content)
                        }
                    })
                    .show()
            }
            R.id.btn_mine_crash -> {

                // 上报错误到 Bugly 上
                CrashReport.postCatchedException(IllegalStateException("are you ok?"))
                // 关闭 Bugly 异常捕捉
                CrashReport.closeBugly()
                throw IllegalStateException("are you ok?")
            }
            R.id.btn_mine_pay -> {

                MessageDialog.Builder(getAttachActivity()!!)
                    .setTitle("捐赠")
                    .setMessage("如果你觉得这个开源项目很棒，希望它能更好地坚持开发下去，可否愿意花一点点钱（推荐 10.24 元）作为对于开发者的激励")
                    .setConfirm("支付宝")
                    .setCancel(null)
                    //.setAutoDismiss(false)
                    .setListener(object : MessageDialog.OnListener {

                        override fun onConfirm(dialog: BaseDialog?) {
                            BrowserActivity.start(getAttachActivity()!!, "https://github.com/getActivity/Donate")
                            toast("AndroidProject 因为有你的支持而能够不断更新、完善，非常感谢支持！")
                            postDelayed({
                                try {
                                    val intent = Intent(Intent.ACTION_VIEW)
                                    intent.data = Uri.parse("alipays://platformapi/startapp?saId=10000007&clientVersion=3.7.0.0718&qrcode=https%3A%2F%2Fqr.alipay.com%2FFKX04202G4K6AVCF5GIY66%3F_s%3Dweb-other")
                                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                    startActivity(intent)
                                } catch (e: ActivityNotFoundException) {
                                    toast("打开支付宝失败，你可能还没有安装支付宝客户端")
                                }
                            }, 2000)
                        }
                    })
                    .show()
            }
        }
    }

    override fun isStatusBarEnabled(): Boolean {
        // 使用沉浸式状态栏
        return !super.isStatusBarEnabled()
    }
}
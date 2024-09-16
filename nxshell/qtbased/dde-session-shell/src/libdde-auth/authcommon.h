// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHCOMMON_H
#define AUTHCOMMON_H

namespace AuthCommon {

/**
 * @brief The AuthFrameFlag enum
 * 认证框架是否可用的标志位
 */
enum AuthFrameFlag {
    Unavailable = 0,        // 认证框架不可用
    Available,              // 认证框架可用
};

/**
 * @brief The AppType enum
 * 发起认证的应用类型
 */
enum AppType {
    None = 0,  // none
    Login = 1, // 登录
    Lock = 2   // 锁屏
};

/**
 * @brief The InputType enum
 * 认证信息输入设备的类型
 */
enum InputType {
    IT_Default = 0, // 默认
    IT_Keyboard,    // 键盘
    IT_Finger,      // 指纹和指静脉
    IT_CameraFace,  // 人脸摄像头
    IT_CameraIris   // 虹膜摄像头
};

/**
 * @brief The AuthType enum
 * 认证类型
 */
enum AuthType {
    AT_None = 0,                 // none
    AT_Password = 1 << 0,        // 密码
    AT_Fingerprint = 1 << 1,     // 指纹
    AT_Face = 1 << 2,            // 人脸
    AT_ActiveDirectory = 1 << 3, // AD域
    AT_Ukey = 1 << 4,            // ukey
    AT_FingerVein = 1 << 5,      // 指静脉
    AT_Iris = 1 << 6,            // 虹膜
    AT_PIN = 1 << 7,             // PIN
    AT_PAM = 1 << 29,            // PAM
    AT_Custom = 1 << 30,         // 自定义
    AT_All = -1                  // all
};

/**
 * @brief The AuthStatus enum
 * 认证状态
 */
enum AuthState {
    AS_None = -1,   // none
    AS_Success,     // 成功，此次认证的最终结果
    AS_Failure,     // 失败，此次认证的最终结果
    AS_Cancel,      // 取消，当认证没有给出最终结果时，调用 End 会出发 Cancel 信号
    AS_Timeout,     // 超时
    AS_Error,       // 错误
    AS_Verify,      // 验证中
    AS_Exception,   // 设备异常，当前认证会被 End
    AS_Prompt,      // 设备提示
    AS_Started,     // 认证已启动，调用 Start 之后，每种成功开启都会发送此信号
    AS_Ended,       // 认证已结束，调用 End 之后，每种成功关闭的都会发送此信号，当某种认证类型被锁定时，也会触发此信号
    AS_Locked,      // 认证已锁定，当认证类型锁定时，触发此信号。该信号不会给出锁定等待时间信息
    AS_Recover,     // 设备恢复，需要调用 Start 重新开启认证，对应 AS_Exception
    AS_Unlocked     // 认证解锁，对应 AS_Locked
};

} // namespace AuthCommon
#endif // AUTHCOMMON_H

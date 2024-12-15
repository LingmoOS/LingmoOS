// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COO_CONSTRANTS_H
#define COO_CONSTRANTS_H

#define COO_SESSION_PORT 51598
#define COO_HARD_PIN "515616"

typedef enum apply_type_t {
    APPLY_LOGIN = 1000,   // 登录认证申请 REQ_LOGIN
    APPLY_INFO = 100,   // 设备信息申请
    APPLY_TRANS = 101,   // 传输申请
    APPLY_TRANS_RESULT = 102,   // 传输申请的结果
    APPLY_SHARE = 111,   // 控制申请
    APPLY_SHARE_RESULT = 112,   // 控制申请的结果
    APPLY_SHARE_STOP = 113,   // 收到停止事件
    APPLY_CANCELED = 120,   // 申请被取消
    APPLY_SCAN_CONNECT = 200,   // 申请扫码连接
    APPLY_PROJECTION = 201,   // 投屏申请
    APPLY_PROJECTION_RESULT = 202,   // 投屏申请的结果
    APPLY_PROJECTION_STOP = 203,   // 收到停止投屏事件
} ApplyReqType;

enum ShareConnectReplyCode {
    SHARE_CONNECT_UNABLE = -1, // 无法连接远端
    SHARE_CONNECT_REFUSE = 0, // 拒绝连接申请
    SHARE_CONNECT_COMFIRM = 1, // 接受连接申请
    SHARE_CONNECT_ERR_CONNECTED = 2, // 连接错误已连接
};

enum ExceptionType {
    EX_NETWORK_PINGOUT = -3, // 远端无法ping通
    EX_SPACE_NOTENOUGH = -2, // 磁盘空间不足
    EX_FS_RWERROR = -1, // 文件读写异常
    EX_OTHER = 0, // 其它异常
};

#endif   // COO_CONSTRANTS_H

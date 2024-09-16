// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOCONSTANTS_H
#define PROTOCONSTANTS_H

#define DATA_SESSION_PORT 51596

typedef enum session_type_t {
    APPLY_LOGIN = 1000, // 认证登录 REQ_LOGIN
    SESSION_LOGIN = 101,   // 登录
    SESSION_TRANS = 102,   // 传输
    SESSION_STOP = 111,   // 中断传输
    SESSION_MESSAGE = 112,   //字符串信息
} SessionReqType;

enum ExceptionType {
    EX_NETWORK_PINGOUT = -3, // 远端无法ping通
    EX_SPACE_NOTENOUGH = -2, // 磁盘空间不足
    EX_FS_RWERROR = -1, // 文件读写异常
    EX_OTHER = 0, // 其它异常
};

#endif   // PROTOCONSTANTS_H

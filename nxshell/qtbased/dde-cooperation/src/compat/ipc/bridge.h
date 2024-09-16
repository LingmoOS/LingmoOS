// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BRIDGE_H
#define BRIDGE_H

#include <co/fastring.h>

typedef enum req_type_t {
    IPC_PING = 10,
    MISC_MSG = 11,
    FRONT_PEER_CB = 100,
    FRONT_CONNECT_CB = 101,
    FRONT_TRANS_STATUS_CB = 102,
    FRONT_FS_PULL_CB = 103,
    FRONT_FS_ACTION_CB = 104,
    FRONT_NOTIFY_FILE_STATUS = 105,
    FRONT_APPLY_TRANS_FILE = 106,
    FRONT_SEND_STATUS = 107,
    FRONT_SERVER_ONLINE = 108,
    FRONT_SHARE_APPLY_CONNECT = 109, // 后端通知被控制方收到连接申请
    FRONT_SHARE_APPLY_CONNECT_REPLY = 110, // 后端通知控制方收到连接申请的结果
    FRONT_SHARE_DISCONNECT = 111, // 收到断开连接
    FRONT_SHARE_START_REPLY = 112, // 后端通知前端共享结果
    FRONT_SHARE_STOP = 113, // 收到停止事件
    FRONT_DISCONNECT_CB = 114, // 断开连接
    FRONT_SHARE_DISAPPLY_CONNECT = 115, // 收到取消申请共享连接
    FRONT_SEARCH_IP_DEVICE_RESULT = 116, // 搜索device的结果
    BACK_GET_DISCOVERY = 200,
    BACK_GET_PEER = 201,
    BACK_GET_PASSWORD = 202,
    BACK_SET_PASSWORD = 203,
    BACK_TRY_CONNECT = 204,
    BACK_GETAPP_CONFIG = 205,
    BACK_SETAPP_CONFIG = 206,
    BACK_TRY_TRANS_FILES = 207,
    BACK_RESUME_JOB = 208,
    BACK_CANCEL_JOB = 209,
    BACK_FS_CREATE = 210,
    BACK_FS_DELETE = 211,
    BACK_FS_RENAME = 212,
    BACK_FS_PULL = 213,
    BACK_DISC_REGISTER = 214,
    BACK_DISC_UNREGISTER = 215,
    BACK_APPLY_TRANS_FILES = 216,
    BACK_SHARE_CONNECT = 217, // 控制方告诉后端连接被控制方
    BACK_SHARE_START = 218, // 控制方告诉后端被控制方开始共享
    BACK_SHARE_DISCONNECT = 219, // 断开连接
    BACK_SHARE_CONNECT_REPLY = 220, // 被控制方告诉后端连接被控制方的回复
    BACK_SHARE_STOP = 221, // 停止共享，两端都可以
    BACK_DISCONNECT_CB = 222, // 文件投送断开连接
    BACK_SHARE_DISAPPLY_CONNECT = 223, // 控制方取消申请共享
    BACK_SEARCH_IP_DEVICE = 224, // 搜索ip目标设备
} ReqType;

typedef enum res_type_t {
    FILE_ENTRY = 500,
    FILE_DIRECTORY = 501,
    GENERIC_RESULT = 502,
    FILE_STATUS = 503,
    CALL_RESULT = 504,
    CONFIG_RESULT = 505,
} ResType;

struct BridgeJsonData {
    uint32_t type; // json数据类型：ReqType or ResType
    fastring json; // json数据结构实例
};

#endif // BRIDGE_H

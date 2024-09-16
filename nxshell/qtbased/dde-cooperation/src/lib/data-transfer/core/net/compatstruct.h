// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATSTRUCT_H
#define COMPATSTRUCT_H

#include <stdlib.h>
#include <string.h>
#include <memory>
#include <functional>

#ifndef PICOJSON_USE_INT64
#define PICOJSON_USE_INT64
#endif
#include "picojson/picojson.h"


namespace ipc {

typedef enum ipc_type_t {
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
} IpcType;



struct GenericResult {
    int32_t id;
    int32_t result;
    std::string msg;
    bool isself;

    void from_json(const picojson::value& _x_) {
    id = static_cast<int32_t>(_x_.get("id").get<double>());
        result = static_cast<int32_t>(_x_.get("result").get<double>());
        msg = _x_.get("msg").get<std::string>();
        isself = _x_.get("isself").get<bool>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["id"] = picojson::value(static_cast<double>(id));
        _x_["result"] = picojson::value(static_cast<double>(result));
        _x_["msg"] = picojson::value(msg);
        _x_["isself"] = picojson::value(isself);
        return picojson::value(_x_);
    }
};


struct MiscJsonCall {
    std::string app;
    std::string json;

    void from_json(const picojson::value& _x_) {
        app = _x_.get("app").get<std::string>();
        json = _x_.get("json").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["app"] = picojson::value(app);
        _x_["json"] = picojson::value(json);
        return picojson::value(_x_);
    }
};


struct SendStatus {
    int32_t type{0};
    int32_t status{0};
    int32_t curstatus{0};
    std::string msg;

    void from_json(const picojson::value& _x_) {
        type = static_cast<int32_t>(_x_.get("type").get<double>());
        status = static_cast<int32_t>(_x_.get("status").get<double>());
        curstatus = static_cast<int32_t>(_x_.get("curstatus").get<double>());
        msg = _x_.get("msg").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["type"] = picojson::value(static_cast<double>(type));
        _x_["status"] = picojson::value(static_cast<double>(status));
        _x_["curstatus"] = picojson::value(static_cast<double>(curstatus));
        _x_["msg"] = picojson::value(msg);
        return picojson::value(_x_);
    }
};

struct SendResult {
    int64_t protocolType;
    int64_t errorType;
    std::string data;

    void from_json(const picojson::value& _x_) {
        protocolType = _x_.get("protocolType").get<int64_t>();
        errorType = _x_.get("errorType").get<int64_t>();
        data = _x_.get("data").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["protocolType"] = picojson::value(protocolType);
        _x_["errorType"] = picojson::value(errorType);
        _x_["data"] = picojson::value(data);
        return picojson::value(_x_);
    }
};

struct FileStatus {
    int32_t job_id;
    int32_t file_id;
    std::string name;
    int32_t status;
    int64_t total;
    int64_t current;
    int64_t millisec;

    void from_json(const picojson::value& _x_) {
        job_id = static_cast<int32_t>(_x_.get("job_id").get<double>());
        file_id = static_cast<int32_t>(_x_.get("file_id").get<double>());
        name = _x_.get("name").get<std::string>();
        status = static_cast<int32_t>(_x_.get("status").get<double>());
        total = static_cast<int64_t>(_x_.get("total").get<double>());
        current = static_cast<int64_t>(_x_.get("current").get<double>());
        millisec = static_cast<int64_t>(_x_.get("millisec").get<double>());
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["job_id"] = picojson::value(static_cast<double>(job_id));
        _x_["file_id"] = picojson::value(static_cast<double>(file_id));
        _x_["name"] = picojson::value(name);
        _x_["status"] = picojson::value(static_cast<double>(status));
        _x_["total"] = picojson::value(static_cast<double>(total));
        _x_["current"] = picojson::value(static_cast<double>(current));
        _x_["millisec"] = picojson::value(static_cast<double>(millisec));
        return picojson::value(_x_);
    }
};




} // ipc

#endif // COMPATSTRUCT_H

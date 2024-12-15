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

inline const char CooperRegisterName[] { "daemon-cooperation" };

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


typedef enum apply_trans_type_t {
    APPLY_TRANS_APPLY = 0, // 请求传输文件
    APPLY_TRANS_CONFIRM = 1, // 接受文件传输
    APPLY_TRANS_REFUSED = 2, // 拒绝文件传输
} ApplyTransType;

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

struct NodePeerInfo {
    std::string proto_version;
    std::string uuid;
    std::string nickname;
    std::string username;
    std::string hostname;
    std::string ipv4;
    std::string share_connect_ip;
    int32_t port;
    int32_t os_type;
    int32_t mode_type;

    void from_json(const picojson::value& _x_) {
        proto_version = _x_.get("proto_version").get<std::string>();
        uuid = _x_.get("uuid").get<std::string>();
        nickname = _x_.get("nickname").get<std::string>();
        username = _x_.get("username").get<std::string>();
        hostname = _x_.get("hostname").get<std::string>();
        ipv4 = _x_.get("ipv4").get<std::string>();
        share_connect_ip = _x_.get("share_connect_ip").get<std::string>();
        port = static_cast<int32_t>(_x_.get("port").get<double>());
        os_type = static_cast<int32_t>(_x_.get("os_type").get<double>());
        mode_type = static_cast<int32_t>(_x_.get("mode_type").get<double>());
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["proto_version"] = picojson::value(proto_version);
        _x_["uuid"] = picojson::value(uuid);
        _x_["nickname"] = picojson::value(nickname);
        _x_["username"] = picojson::value(username);
        _x_["hostname"] = picojson::value(hostname);
        _x_["ipv4"] = picojson::value(ipv4);
        _x_["share_connect_ip"] = picojson::value(share_connect_ip);
        _x_["port"] = picojson::value(static_cast<double>(port));
        _x_["os_type"] = picojson::value(static_cast<double>(os_type));
        _x_["mode_type"] = picojson::value(static_cast<double>(mode_type));
        return picojson::value(_x_);
    }
};

struct AppPeerInfo {
    std::string appname;
    std::string json;

    void from_json(const picojson::value& _x_) {
        appname = _x_.get("appname").get<std::string>();
        json = _x_.get("json").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["appname"] = picojson::value(appname);
        _x_["json"] = picojson::value(json);
        return picojson::value(_x_);
    }
};

struct NodeInfo {
    NodePeerInfo os;
    std::vector<AppPeerInfo> apps;

    void from_json(const picojson::value& _x_) {
        os.from_json(_x_.get("os"));

        if (_x_.get("apps").is<picojson::array>()) {
            const picojson::array& app_array = _x_.get("apps").get<picojson::array>();
            for (const auto& app_val : app_array) {
                AppPeerInfo app;
                app.from_json(app_val);
                apps.push_back(std::move(app));
            }
        }
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["os"] = os.as_json();
        picojson::array app_array;
        for (const auto& app : apps) {
            app_array.push_back(app.as_json());
        }
        _x_["apps"] = picojson::value(app_array);
        return picojson::value(_x_);
    }
};

struct NodeList {
    int32_t code;
    std::vector<NodeInfo> peers;

    void from_json(const picojson::value& _x_) {
        code = static_cast<int32_t>(_x_.get("code").get<double>());
        const picojson::array& peers_array = _x_.get("peers").get<picojson::array>();
        for (const auto& peer_val : peers_array) {
            NodeInfo peer;
            peer.from_json(peer_val);
            peers.emplace_back(std::move(peer));
        }
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["code"] = picojson::value(static_cast<double>(code));
        picojson::array peers_array;
        for (const auto& peer : peers) {
            peers_array.push_back(peer.as_json());
        }
        _x_["peers"] = picojson::value(peers_array);
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

struct ApplyTransFiles {
    std::string machineName;
    std::string appname;
    std::string tarAppname;
    int32_t type;
    std::string selfIp;
    int32_t selfPort;

    void from_json(const picojson::value& _x_) {
        machineName = _x_.get("machineName").get<std::string>();
        appname = _x_.get("appname").get<std::string>();
        tarAppname = _x_.get("tarAppname").get<std::string>();
        type = static_cast<int32_t>(_x_.get("type").get<double>());
        selfIp = _x_.get("selfIp").get<std::string>();
        selfPort = static_cast<int32_t>(_x_.get("selfPort").get<double>());
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["machineName"] = picojson::value(machineName);
        _x_["appname"] = picojson::value(appname);
        _x_["tarAppname"] = picojson::value(tarAppname);
        _x_["type"] = picojson::value(static_cast<double>(type));
        _x_["selfIp"] = picojson::value(selfIp);
        _x_["selfPort"] = picojson::value(static_cast<double>(selfPort));
        return picojson::value(_x_);
    }
};

struct ShareEvents {
    uint32_t eventType;
    std::string data;

    void from_json(const picojson::value& _x_) {
        eventType = static_cast<uint32_t>(_x_.get("eventType").get<double>());
        data = _x_.get("data").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["eventType"] = picojson::value(static_cast<double>(eventType));
        _x_["data"] = picojson::value(data);
        return picojson::value(_x_);
    }
};

struct ShareConnectApply {
    std::string appName;
    std::string tarAppname;
    std::string ip;
    std::string tarIp;
    std::string data;

    void from_json(const picojson::value& _x_) {
        appName = _x_.get("appName").get<std::string>();
        tarAppname = _x_.get("tarAppname").get<std::string>();
        ip = _x_.get("ip").get<std::string>();
        tarIp = _x_.get("tarIp").get<std::string>();
        data = _x_.get("data").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["appName"] = picojson::value(appName);
        _x_["tarAppname"] = picojson::value(tarAppname);
        _x_["ip"] = picojson::value(ip);
        _x_["tarIp"] = picojson::value(tarIp);
        _x_["data"] = picojson::value(data);
        return picojson::value(_x_);
    }
};

struct ShareConnectReply {
    std::string appName;
    std::string tarAppname;
    std::string msg;
    std::string ip;
    int32_t reply;

    void from_json(const picojson::value& _x_) {
        appName = _x_.get("appName").get<std::string>();
        tarAppname = _x_.get("tarAppname").get<std::string>();
        msg = _x_.get("msg").get<std::string>();
        ip = _x_.get("ip").get<std::string>();
        reply = static_cast<int32_t>(_x_.get("reply").get<double>());
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["appName"] = picojson::value(appName);
        _x_["tarAppname"] = picojson::value(tarAppname);
        _x_["msg"] = picojson::value(msg);
        _x_["ip"] = picojson::value(ip);
        _x_["reply"] = picojson::value(static_cast<double>(reply));
        return picojson::value(_x_);
    }
};

struct ShareDisConnect {
    std::string appName;
    std::string tarAppname;
    std::string msg;

    void from_json(const picojson::value& _x_) {
        appName = _x_.get("appName").get<std::string>();
        tarAppname = _x_.get("tarAppname").get<std::string>();
        msg = _x_.get("msg").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["appName"] = picojson::value(appName);
        _x_["tarAppname"] = picojson::value(tarAppname);
        _x_["msg"] = picojson::value(msg);
        return picojson::value(_x_);
    }
};

struct ShareConnectDisApply {
    std::string appName;
    std::string tarAppname;
    std::string ip;
    std::string msg;

    void from_json(const picojson::value& _x_) {
        appName = _x_.get("appName").get<std::string>();
        tarAppname = _x_.get("tarAppname").get<std::string>();
        ip = _x_.get("ip").get<std::string>();
        msg = _x_.get("msg").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["appName"] = picojson::value(appName);
        _x_["tarAppname"] = picojson::value(tarAppname);
        _x_["ip"] = picojson::value(ip);
        _x_["msg"] = picojson::value(msg);
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


struct SearchDevice {
    std::string app;
    std::string ip;
    bool remove{ false };

    void from_json(const picojson::value& _x_) {
        app = _x_.get("app").get<std::string>();
        ip = _x_.get("ip").get<std::string>();
        remove = _x_.get("remove").get<bool>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["app"] = picojson::value(app);
        _x_["ip"] = picojson::value(ip);
        _x_["remove"] = picojson::value(remove);
        return picojson::value(_x_);
    }
};

struct SearchDeviceResult {
    bool result;
    std::string msg;

    void from_json(const picojson::value& _x_) {
        result = _x_.get("result").get<bool>();
        msg = _x_.get("msg").get<std::string>();
    }

    picojson::value as_json() const {
        picojson::object _x_;
        _x_["result"] = picojson::value(result);
        _x_["msg"] = picojson::value(msg);
        return picojson::value(_x_);
    }
};


} // ipc

#endif // COMPATSTRUCT_H

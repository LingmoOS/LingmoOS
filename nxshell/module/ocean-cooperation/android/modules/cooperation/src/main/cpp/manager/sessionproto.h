// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONPROTO_H
#define SESSIONPROTO_H

#include <stdlib.h>
#include <string.h>
#include <memory>
#include <functional>

#ifndef PICOJSON_USE_INT64
#define PICOJSON_USE_INT64
#endif
#include "picojson/picojson.h"


#define SESSION_TCP_PORT  51616
#define WEB_TCP_PORT  SESSION_TCP_PORT + 2

enum LoginStatus {
    LOGIN_SUCCESS = 666,
    LOGIN_DENIED = 444
};

enum CallResult {
    DO_SUCCESS,
    DO_FAILED
};

typedef enum dt_type_t {
    REQ_LOGIN = 1000,
    REQ_FREE_SPACE = 1001,
    REQ_TRANS_DATAS = 1002,
    REQ_TRANS_CANCLE = 1003,
    CAST_INFO = 1004,
    INFO_TRANS_COUNT = 1005,
} ComType;

typedef enum apply_flag_t {
    ASK_NEEDCONFIRM = 10,
    ASK_QUIET = 12,
    ASK_CANCELED = 14,
    DO_WAIT = 20,
    DO_DONE = 22,
    REPLY_ACCEPT = 30,
    REPLY_REJECT = 32,
} ApplyFlag;


enum CallbackMask {
    CONN_CHANGED = 0x1,
    RPC_RESULT = 0x2,
};

using ExtenMessageHandler = std::function<bool(int32_t mask, const picojson::value &json_value, std::string *res_msg)>;

using CallBackHandler = std::function<void(int32_t mask, int &result, const std::string &msg)>;

class SessionCallback : public std::enable_shared_from_this<SessionCallback>
{
public:
    virtual void onConnectChanged(int result, const std::string &reason) = 0;

    virtual void onRpcResult(int type, const std::string &response) = 0;
};

struct LoginMessage {
    std::string name;
    std::string auth;

    void from_json(const picojson::value& _x_) {
        name = _x_.get("name").to_str();
        auth = _x_.get("auth").to_str();
    }

    picojson::value as_json() const {
        picojson::object obj;
        obj["name"] = picojson::value(name);
        obj["auth"] = picojson::value(auth);
        return picojson::value(obj);
    }
};

struct FreeSpaceMessage {
    int64_t total;
    int64_t free;

    void from_json(const picojson::value& _x_) {
        total = _x_.get("total").get<int64_t>();
        free = _x_.get("free").get<int64_t>();
    }

    picojson::value as_json() const {
        picojson::object obj;
        obj["total"] = picojson::value(total);
        obj["free"] = picojson::value(free);
        return picojson::value(obj);
    }
};

struct TransDataMessage {
    std::string id; // the file serve id.
    std::vector<std::string> names; // the file/dir name list
    std::string endpoint; // ip:port:token
    bool flag;  // request: dir or file;  response: ready to receive or error(not enough space)
    int64_t size;  // the space need, total folder size or all file size

    void from_json(const picojson::value& _x_) {
        id = _x_.get("id").to_str();
        endpoint = _x_.get("token").to_str();
        flag = _x_.get("flag").get<bool>();
        size = _x_.get("size").get<int64_t>();

        if (_x_.get("names").is<picojson::array>()) {
            const picojson::array &namesArr = _x_.get("names").get<picojson::array>();
            for (const auto &elem : namesArr) {
                if (elem.is<std::string>()) {
                    names.push_back(elem.get<std::string>());
                }
            }
        }
    }

    picojson::value as_json() const {
        picojson::object obj;
        obj["id"] = picojson::value(id);
        obj["token"] = picojson::value(endpoint);
        obj["flag"] = picojson::value(flag);
        obj["size"] = picojson::value(size);

        picojson::array namesArr;
        for (const auto &name : names) {
            namesArr.push_back(picojson::value(name));
        }
        obj["names"] = picojson::value(namesArr);
        return picojson::value(obj);
    }
};

struct TransCancelMessage {
    std::string id; // the file serve id.
    std::string name; // the file/dir name
    std::string reason; // user or io error

    void from_json(const picojson::value& _x_) {
        id = _x_.get("id").to_str();
        name = _x_.get("name").to_str();
        reason = _x_.get("reason").to_str();
    }

    picojson::value as_json() const {
        picojson::object obj;
        obj["id"] = picojson::value(id);
        obj["name"] = picojson::value(name);
        obj["reason"] = picojson::value(reason);
        return picojson::value(obj);
    }
};

struct ApplyMessage {
    int64_t flag {0};
    std::string  nick;
    std::string  host;
    int64_t port {0};
    std::string  fingerprint {""};

    void from_json(const picojson::value& _x_) {
        flag = _x_.get("flag").get<int64_t>();
        nick = _x_.get("nick").to_str();
        host = _x_.get("selfIp").to_str();
        port = _x_.get("selfPort").get<int64_t>();
        if (_x_.contains("fingerprint"))
            fingerprint = _x_.get("fingerprint").to_str();
        else
            fingerprint = "";
    }

    picojson::value as_json() const {
        picojson::object obj;
        obj["flag"] = picojson::value(flag);
        obj["nick"] = picojson::value(nick);
        obj["selfIp"] = picojson::value(host);
        obj["selfPort"] = picojson::value(port);
        obj["fingerprint"] = picojson::value(fingerprint);
        return picojson::value(obj);
    }
};

#endif // SESSIONPROTO_H

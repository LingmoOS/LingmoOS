// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionhelper.h"
#include <iostream>
#include <memory>

#include "utils.h"

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

// 此函数位于Java线程
SessionHelper &SessionHelper::getInstance() {
    static SessionHelper instance;
    return instance;
}

// 此函数位于Java的
int SessionHelper::init(JNIEnv *env, jobject object, jstring ip)
{
    if (nullptr != env and nullptr != object) {
        // 初始化JavaVM对象，全局唯一
        env->GetJavaVM(std::addressof(_jvm));
        _jobj = env->NewGlobalRef(object);
        _jclz = env->GetObjectClass(_jobj);
        _onConnectCallBack = env->GetMethodID(_jclz, "onConnectChanged",
                                        "(ILjava/lang/String;)V");
        _onRpcResultCallBack = env->GetMethodID(_jclz, "onAsyncRpcResult",
                                               "(ILjava/lang/String;)V");

        const char *chars = env->GetStringUTFChars(ip, nullptr);
        std::string ipStr(chars);
        initManager(ipStr);
        env->ReleaseStringUTFChars(ip, chars);
        return 0;
    }
    return -1;
}

// 此函数位于C++创建的线程
void SessionHelper::callbackConnectChanged(int result, const std::string &reason)
{
    // Java虚拟机对象不允许线程共享，当前线程获取，通过反射回调Java方法
    JNIEnv *env = nullptr;
    _jvm->AttachCurrentThread(std::addressof(env), nullptr);

    jstring resStr = env->NewStringUTF(reason.data());

    // 回调方法
    env->CallVoidMethod(_jobj, _onConnectCallBack, (jint)result, resStr);

    // 在当前线程释放虚拟机对象
    _jvm->DetachCurrentThread();
}

void SessionHelper::callbackRpcResult(int type, const std::string &response)
{
    // Java虚拟机对象不允许线程共享，当前线程获取，通过反射回调Java方法
    JNIEnv *env = nullptr;
    _jvm->AttachCurrentThread(std::addressof(env), nullptr);

    jstring resStr= env->NewStringUTF(response.data());

    // 回调方法
    env->CallVoidMethod(_jobj, _onRpcResultCallBack, (jint)type, resStr);

    // 在当前线程释放虚拟机对象
    _jvm->DetachCurrentThread();
}

SessionHelper::SessionHelper()
{
}

void SessionHelper::initManager(std::string const &ip)
{
    _serveIp = std::string(ip);
    _sessionManager = std::make_shared<SessionManager>(_serveIp, COO_SESSION_PORT);

    ExtenMessageHandler msg_cb([this](int32_t mask, const picojson::value &json_value, std::string *res_msg) -> bool {
        switch (mask) {
        case APPLY_INFO: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_DONE;
            // response my device info.
            res.nick = _deviceName;
            *res_msg = res.as_json().serialize();
        }
            return true;
        case APPLY_TRANS: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_WAIT;
            *res_msg = res.as_json().serialize();
            _confirmTargetAddress = std::string(req.host);
        }
            return true;
        case APPLY_TRANS_RESULT: {
            ApplyMessage req, res;
            req.from_json(json_value);
            bool agree = (req.flag == REPLY_ACCEPT);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
        }
            return true;
        case APPLY_PROJECTION: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_WAIT;
            std::string info = std::string(req.host + "," + req.nick + "," + req.fingerprint);
            *res_msg = res.as_json().serialize();
            _confirmTargetAddress = std::string(req.host);
        }
            return true;
        case APPLY_PROJECTION_RESULT: {
            ApplyMessage req, res;
            req.from_json(json_value);
            bool agree = (req.flag == REPLY_ACCEPT);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
        }
            return true;
        case APPLY_PROJECTION_STOP: {
            ApplyMessage req, res;
            req.from_json(json_value);
            res.flag = DO_DONE;
            *res_msg = res.as_json().serialize();
        }
            return true;
        }

        // unhandle message
        return false;
    });

    CallBackHandler cb_cb([this](int32_t mask, int &result, const std::string &msg) -> void {
        switch (mask) {
            case CONN_CHANGED: {
                this->callbackConnectChanged(result, msg);
            }
                break;
            case RPC_RESULT: {
                this->callbackRpcResult(result, msg);
            }
                break;
        }
    });
    _sessionManager->setJavaCallback(cb_cb);

    _sessionManager->setSessionExtCallback(msg_cb);
    // _sessionManager->updatePin(COO_HARD_PIN);
    // _sessionManager->sessionListen();
}

// 设置存储文件夹
void SessionHelper::setStoragePath(const std::string &root, const std::string &folderName)
{
    if (_sessionManager) {
        _sessionManager->setStorageRoot(root);
        _sessionManager->updateSaveFolder(folderName);
    }
}

// 设置设备名称
void SessionHelper::setDeviceName(const std::string &name)
{    
    _deviceName = name;
    if (_sessionManager) {
        _sessionManager->updateNick(_deviceName);
    }
}


// 异步连接请求
int SessionHelper::asyncConnect(const std::string &ip, int port, const std::string &pin)
{
    _confirmTargetAddress = ip; // 设置目标地址
    // 在这里实现连接逻辑，通常为异步操作
    return _sessionManager->sessionConnect(ip, port, pin);
}

// 异步断开请求
void SessionHelper::asyncDisconnect()
{
    _sessionManager->sessionDisconnect(_confirmTargetAddress);

    _confirmTargetAddress.clear(); // 清空目标地址
}

// 扫码连接请求
void SessionHelper::requestConnect(const std::string &resolution)
{
    // 在这里实现扫码连接的逻辑
    std::cout << "Requesting requestConnect " << std::endl;

    ApplyMessage msg;
    msg.flag = ASK_QUIET;
    msg.nick = _deviceName + "=" + std::string(resolution);
    msg.host = _serveIp;
    msg.fingerprint = _selfFingerPrint; // send self fingerprint
    std::string jsonMsg = msg.as_json().serialize();
    _sessionManager->sendRpcRequest(_confirmTargetAddress, APPLY_SCAN_CONNECT, jsonMsg);
}

// 投影请求
void SessionHelper::requestProjection(const std::string &myNick, int vncPort)
{
    // 在这里实现投影请求的逻辑
    std::cout << "Requesting projection on port " << vncPort << std::endl;

    ApplyMessage msg;
    msg.flag = ASK_NEEDCONFIRM;
    msg.nick = myNick;
    msg.host = _serveIp + ":" + std::to_string(vncPort);
    msg.fingerprint = _selfFingerPrint; // send self fingerprint
    std::string jsonMsg = msg.as_json().serialize();
    _sessionManager->sendRpcRequest(_confirmTargetAddress, APPLY_PROJECTION, jsonMsg);
}

// 停止投影请求
void SessionHelper::requestStopProjection(const std::string &myNick)
{
    // 在这里实现停止投影的逻辑
    std::cout << "Stopping projection." << std::endl;

    ApplyMessage msg;
    msg.flag = ASK_QUIET;
    msg.nick = myNick;
    msg.host = _serveIp;
    msg.fingerprint = _selfFingerPrint; // send self fingerprint
    std::string jsonMsg = msg.as_json().serialize();
    _sessionManager->sendRpcRequest(_confirmTargetAddress, APPLY_PROJECTION_STOP, jsonMsg);
}

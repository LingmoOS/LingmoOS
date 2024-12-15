// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANDROID_SESSIONHELPER_H
#define ANDROID_SESSIONHELPER_H

#include <jni.h>
#include <string_view>
#include "manager/sessionmanager.h"

class SessionHelper final {

public:
    static SessionHelper &getInstance();

    int init(JNIEnv *, jobject, jstring);

    // setting
    void setStoragePath(const std::string &root, const std::string &folderName);
    void setDeviceName(const std::string &name);

    // connect/disconnect request
    int asyncConnect(const std::string &ip, int port, const std::string &pin);
    void asyncDisconnect();

    // projection request
    void requestConnect(const std::string &resolution);
    void requestProjection(const std::string &myNick, int vncPort);
    void requestStopProjection(const std::string &myNick);

private:
    explicit SessionHelper();
    ~SessionHelper() = default;

    void initManager(std::string const &ip);
    void callbackConnectChanged(int result, const std::string &reason);
    void callbackRpcResult(int type, const std::string &response);

private:
    std::shared_ptr<SessionManager> _sessionManager;

    std::string _confirmTargetAddress {""};   // remote ip address

    std::string _selfFingerPrint = {};
    std::string _serveIp = {};
    std::string _deviceName = {};

    JavaVM *_jvm;  // Java虚拟机对象
    jobject _jobj; // Java类对象
    jclass _jclz;  // 字节码对象
    jmethodID _onConnectCallBack;
    jmethodID _onRpcResultCallBack;
};


#endif //ANDROID_SESSIONHELPER_H

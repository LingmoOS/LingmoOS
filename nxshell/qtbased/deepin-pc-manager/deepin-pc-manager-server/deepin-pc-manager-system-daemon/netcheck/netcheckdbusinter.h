// Copyright (C) 2019 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "netcheck_adaptor.h"

#include <QObject>
#include <QThread>
#include <QDBusConnection>

#define NET_CHECK_DBUS_NAME "com.deepin.pc.manager.netcheck"
#define NET_CHECK_DBUS_PATH "/com/deepin/pc/manager/netcheck"

class QDBusContext;
class NetCheckSysSrvModel;

class NetCheckDBusInter : public QObject
    , protected QDBusContext
{
    Q_OBJECT
public:
    Q_PROPERTY(bool Availabled READ availabled)
    explicit NetCheckDBusInter(QObject *parent = nullptr);
    virtual ~NetCheckDBusInter() override;

public Q_SLOTS:
    void StartApp();
    void ExitApp();

    // 是否可用
    bool availabled() { return true; }

    // 网络硬件检测
    void StartCheckNetDevice();
    // 网络连接配置检测
    void StartCheckNetConnSetting();
    // DHCP检测
    void StartCheckNetDHCP();
    // DNS检测
    void StartCheckNetDNS();
    // host检测
    void StartCheckNetHost();
    // 网络连接访问检测
    void StartCheckNetConn(QString url);

    // 获取网络检测状态
    bool GetNetCheckStatus();
    void SetNetCheckStatus(bool status);

    // 接收网络硬件检测结果
    void acceptCheckNetDevice(int result);
    // 接收网络连接配置检测结果
    void acceptCheckNetConnSetting(int result);
    // 接收DHCP检测结果
    void acceptCheckNetDHCP(int result);
    // 接收DNS检测结果
    void acceptCheckNetDNS(int result);
    // 接收host检测结果
    void acceptCheckNetHost(int result);
    // 接收网络连接访问结果
    void acceptCheckNetConn(int result);

private:
    // 校验调用者
    bool isValidInvoker();

Q_SIGNALS:
    // 发送网络硬件检测
    void sendCheckNetDevice();
    // 发送网络连接配置检测
    void sendCheckNetConnSetting();
    // 发送DHCP检测
    void sendCheckNetDHCP();
    // 发送DNS检测
    void sendCheckNetDNS();
    // 发送host检测
    void sendCheckNetHost();
    // 发送网络连接访问测
    void sendCheckNetConn(QString url);

    // 发送网络硬件检测结果
    void NotifyCheckNetDevice(int result);
    // 发送网络连接配置检测结果
    void NotifyCheckNetConnSetting(int result);
    // 发送DHCP检测结果
    void NotifyCheckNetDHCP(int result);
    // 发送DNS检测结果
    void NotifyCheckNetDNS(int result);
    // 发送host检测结果
    void NotifyCheckNetHost(int result);
    // 发送网络连接访问检测结果
    void NotifyCheckNetConn(int result);

private:
    NetcheckAdaptor *m_adaptor;
    // 文件审计
    QThread *m_netCheckSrvModelThread;
    NetCheckSysSrvModel *m_netCheckModel;

    bool m_isNetChecking = false;
};

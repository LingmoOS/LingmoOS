// Copyright (C) 2019 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QFile>
#include <QVariant>
#include <QDBusObjectPath>

class DBusInvokerInterface;
class NetCheckSysSrvModel : public QObject
{
    Q_OBJECT
public:
    explicit NetCheckSysSrvModel(QObject *parent = nullptr);
    ~NetCheckSysSrvModel();

private:
    bool checkNetConnSetting();
    quint32 ipv4StringToInteger(const QString &ip);
    int subMaskStringToInteger(const QString &ip);
    bool pingCheck(QString url);
    bool nslookupDNS(QString dns);
    bool checkHostLineFormat(QString needCheck, QString headReg, QStringList list);

public Q_SLOTS:
    void initData();

    // 开始网络硬件检测
    void startCheckNetDevice();
    // 开始网络连接配置检测
    void startCheckNetConnSetting();
    // 开始DHCP检测
    void startCheckNetDHCP();
    // 开始DNS检测
    void startCheckNetDNS();
    // 开始host检测
    void startCheckNetHost();
    // 开始网络连接访问
    void startCheckNetConn(QString url);

Q_SIGNALS:
    // 发送网络硬件检测结果
    void sendCheckNetDevice(int result);
    // 发送网络连接配置检测结果
    void sendCheckNetConnSetting(int result);
    // 发送DHCP检测结果
    void sendCheckNetDHCP(int result);
    // 发送DNS检测结果
    void sendCheckNetDNS(int result);
    // 发送host检测结果
    void sendCheckNetHost(int result);
    // 发送网络连接访问结果
    void sendCheckNetConn(int result);

private:
    DBusInvokerInterface *m_netWorkManagerDBusInter;
    DBusInvokerInterface *m_netWorkDBusInter;

    QList<QDBusObjectPath> m_allServicePaths;
    bool m_netConnStatus = false;
    bool m_netAutoConnStatus = false;
    QString m_primaryConnPath;
    QString m_settingConnPath;
    QMap<QString, QMap<QString, QVariant>> m_settings;
    QString m_dhcp4ConnPath;
    QString m_ip4ConfigConnPath;
};

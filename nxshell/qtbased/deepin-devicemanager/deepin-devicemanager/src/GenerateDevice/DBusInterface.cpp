// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusInterface.h"
#include "DDLog.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

using namespace DDLog;

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusInterface *> DBusInterface::s_Instance;
std::mutex DBusInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceInfo";
const QString DEVICE_SERVICE_PATH = "/org/deepin/DeviceInfo";
const QString DEVICE_SERVICE_INTERFACE = "org.deepin.DeviceInfo";

DBusInterface::DBusInterface()
    : mp_Iface(nullptr)
{
    // 初始化dbus
    init();
}

bool DBusInterface::getInfo(const QString &key, QString &info)
{
    // 调用dbus接口获取设备信息
    QDBusReply<QString> reply = mp_Iface->call("getInfo", key);
    if (reply.isValid()) {
        info = reply.value();
        return true;
    } else {
        qCInfo(appLog) << "Error in getting info from getInfo .......................................";
        return false;
    }
}

void DBusInterface::refreshInfo()
{
    mp_Iface->asyncCall("refreshInfo");
}

void DBusInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, DEVICE_SERVICE_PATH, DEVICE_SERVICE_INTERFACE, QDBusConnection::systemBus());
}

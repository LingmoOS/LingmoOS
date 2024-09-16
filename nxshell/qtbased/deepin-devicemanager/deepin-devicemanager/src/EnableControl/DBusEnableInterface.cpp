// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusEnableInterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusEnableInterface *> DBusEnableInterface::s_Instance;
std::mutex DBusEnableInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceControl";
const QString ENABLE_SERVICE_PATH = "/org/deepin/DeviceControl";
const QString ENABLE_SERVICE_INTER = "org.deepin.DeviceControl";

DBusEnableInterface::DBusEnableInterface()
    : mp_Iface(nullptr)
{
    // 初始化dbus
    init();
}

bool DBusEnableInterface::getRemoveInfo(QString &info)
{
    // 调用dbus接口获取设备信息
    QDBusReply<QString> reply = mp_Iface->call("getRemoveInfo");
    if (reply.isValid()) {
        info = reply.value();
        return true;
    } else {
        return false;
    }
}

bool DBusEnableInterface::getAuthorizedInfo(QString &info)
{
    QDBusReply<QString> reply = mp_Iface->call("getAuthorizedInfo");
    if (reply.isValid()) {
        info = reply.value();
        return true;
    } else {
        info = "";
        return false;
    }
}

bool DBusEnableInterface::isDeviceEnabled(const QString &unique_id)
{
    QDBusReply<bool> reply = mp_Iface->call("isDeviceEnabled", unique_id);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

bool DBusEnableInterface::enable(const QString &hclass, const QString &name, const QString &path, const QString &value, bool enable_device, const QString &strDriver)
{
    QDBusReply<bool> reply = mp_Iface->call("enable", hclass, name, path, value, enable_device, strDriver);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

bool DBusEnableInterface::enablePrinter(const QString &hclass, const QString &name, const QString &path, bool enable_device)
{
    QDBusReply<bool> reply = mp_Iface->call("enablePrinter", hclass, name, path, enable_device);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

void DBusEnableInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, ENABLE_SERVICE_PATH, ENABLE_SERVICE_INTER, QDBusConnection::systemBus());
}

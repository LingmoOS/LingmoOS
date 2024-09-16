// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controlinterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<ControlInterface *> ControlInterface::s_instance;
std::mutex ControlInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceControl";
const QString ENABLE_SERVICE_PATH = "/org/deepin/DeviceControl";
const QString ENABLE_SERVICE_INTER = "org.deepin.DeviceControl";

ControlInterface::ControlInterface()
    : m_iface(nullptr)
{
    // 初始化dbus
    init();
}

void ControlInterface::disableOutDevice(const QString &devInfo)
{
    // 调用dbus接口获取设备信息
    if (m_iface != nullptr && m_iface->isValid()) {
        m_iface->call("disableOutDevice", devInfo);
    }
}

void ControlInterface::disableInDevice()
{
    // 调用dbus接口获取设备信息
    if (m_iface != nullptr && m_iface->isValid()) {
        m_iface->call("disableInDevice");
    }
}

void ControlInterface::updateWakeup(const QString &devInfo)
{
    // 调用dbus接口获取设备信息
    if (m_iface != nullptr && m_iface->isValid()) {
        m_iface->call("updateWakeup", devInfo);
    }
}

void ControlInterface::setMonitorWorkingDBFlag(bool flag)
{
    // 调用dbus接口获取设备信息
    if (m_iface != nullptr && m_iface->isValid()) {
        m_iface->call("setMonitorWorkingDBFlag", flag);
    }
}

bool ControlInterface::monitorWorkingDBFlag()
{
    // 调用dbus接口获取设备信息
    if (m_iface != nullptr && m_iface->isValid()) {
        QDBusReply<bool> reply = m_iface->call("monitorWorkingDBFlag");
        if (reply.isValid()) {
            return reply.value();
        }
    }
    return true;
}

void ControlInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    m_iface = new QDBusInterface(SERVICE_NAME, ENABLE_SERVICE_PATH, ENABLE_SERVICE_INTER, QDBusConnection::systemBus());
//    QDBusConnection::systemBus().connect(SERVICE_NAME, ENABLE_SERVICE_PATH, ENABLE_SERVICE_INTER, "sigFinished", this, SIGNAL(sigFinished(bool, QString)));
//    QDBusConnection::systemBus().connect(SERVICE_NAME, ENABLE_SERVICE_PATH, ENABLE_SERVICE_INTER, "sigUpdate", this, SIGNAL(sigUpdate()));
}

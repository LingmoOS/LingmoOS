// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusWakeupInterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QFile>
#include <QMetaMethod>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusWakeupInterface *> DBusWakeupInterface::s_Instance;
std::mutex DBusWakeupInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceControl";
const QString WAKEUP_SERVICE_PATH = "/org/deepin/DeviceControl";
const QString WAKEUP_INTERFACE = "org.deepin.DeviceControl";
#ifdef OS_BUILD_V23
const QString INPUT_SERVICE_NAME = "org.deepin.dde.InputDevices1";
const QString INPUT_WAKEUP_SERVICE_PATH = "/org/deepin/dde/InputDevices1";
const QString INPUT_WAKEUP_INTERFACE = "org.deepin.dde.InputDevices1";
#else
const QString INPUT_SERVICE_NAME = "com.deepin.system.InputDevices";
const QString INPUT_WAKEUP_SERVICE_PATH = "/com/deepin/system/InputDevices";
const QString INPUT_WAKEUP_INTERFACE = "com.deepin.system.InputDevices";
#endif
const QString INPUT_WAKEUP_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";

DBusWakeupInterface::DBusWakeupInterface()
    : mp_Iface(nullptr), mp_InputIface(nullptr)
{
    init();
}

bool DBusWakeupInterface::setWakeupMachine(const QString &unique_id, const QString &path, bool wakeup, const QString &name)
{
    if (nullptr != mp_InputIface && mp_InputIface->isValid()) {
        QStringList pathList = path.split("/", QString::SkipEmptyParts);
        if (pathList.size() < 3)
            return false;

        if (name.contains("PS/2")) {
            // ps2设备无法通过/sys/devices/platform/i8042/serio1/power/wakeup控制，只能通过acpi的接口进行控制
            QDBusInterface interface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
            if (interface.isValid()) {
                QDBusMessage replay = interface.call("Get", INPUT_WAKEUP_INTERFACE, "SupportWakeupDevices");
                QVariant v =  replay.arguments().first();
                if (v.isValid()) {
                    QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
                    QMap<QString, QString> allSupportWakeupDevices;
                    arg >> allSupportWakeupDevices;
                    QStringList pathList = allSupportWakeupDevices.keys();

                    for (QString path : pathList) {
                        if (path.contains("PS2")) {
                            mp_InputIface->call("SetWakeupDevices", path, wakeup ? "enabled" : "disabled");
                            return true;
                        }
                    }
                }
            }
        } else {
            auto metaObject = mp_InputIface->metaObject();
            for (int i = 0 ; i < metaObject->methodCount(); ++i) {
                if (metaObject->method(i).name() == "SetWakeupDevices") {
                    QString curPath = pathList[pathList.size() - 2];
                    QString busPath = QString("/sys/bus/usb/devices/%1/power/wakeup").arg(curPath);
                    mp_InputIface->call("SetWakeupDevices", busPath, wakeup ? "enabled" : "disabled");
                    return true;
                }
            }
        }
    }

    QDBusReply<bool> reply = mp_Iface->call("setWakeupMachine", unique_id, path, wakeup);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

bool DBusWakeupInterface::canInputWakeupMachine(const QString &path)
{
    if (nullptr != mp_InputIface && mp_InputIface->isValid()) {
        QDBusInterface interface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        if (interface.isValid()) {
            QDBusMessage replay = interface.call("Get", INPUT_WAKEUP_INTERFACE, "SupportWakeupDevices");
            QVariant v =  replay.arguments().first();
            if (v.isValid()) {
                QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
                QMap<QString, QString> allSupportWakeupDevices;
                arg >> allSupportWakeupDevices;

                QString curPath = path.left(path.size() - 13);
                int index = curPath.lastIndexOf('/');
                if (index < 1) {
                    return false;
                }
                curPath = curPath.right(curPath.size() - index - 1);
                QString busPath = QString("/sys/bus/usb/devices/%1/power/wakeup").arg(curPath);
                return allSupportWakeupDevices.contains(busPath);
            }
        }
    }

    QFile file(path);
    return file.open(QIODevice::ReadOnly);
}

bool DBusWakeupInterface::isInputWakeupMachine(const QString &path, const QString &name)
{
    if (nullptr != mp_InputIface && mp_InputIface->isValid()) {
        QDBusInterface interface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        if (interface.isValid()) {
            QDBusMessage replay = interface.call("Get", INPUT_WAKEUP_INTERFACE, "SupportWakeupDevices");
            QVariant v =  replay.arguments().first();
            if (v.isValid()) {
                QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
                QMap<QString, QString> allSupportWakeupDevices;
                arg >> allSupportWakeupDevices;

                if (name.contains("PS/2")) {
                    for(QString path : allSupportWakeupDevices.keys()) {
                        if (path.contains("PS2")) {
                            return allSupportWakeupDevices[path] == "enabled";
                        }
                    }
                } else {
                    QString curPath = path.left(path.size() - 13);
                    int index = curPath.lastIndexOf('/');
                    if (index < 1)
                        return false;
                    curPath = curPath.right(curPath.size() - index - 1);
                    QString busPath = QString("/sys/bus/usb/devices/%1/power/wakeup").arg(curPath);
                    return (allSupportWakeupDevices.contains(busPath) && allSupportWakeupDevices[busPath] == "enabled");
                }
            }
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QString info = file.readAll();
    return !(info.contains("disabled"));
}

int DBusWakeupInterface::isNetworkWakeup(const QString &logical_name)
{
    QDBusReply<int> reply = mp_Iface->call("isNetworkWakeup", logical_name);
    if (reply.isValid()) {
        return reply.value();
    }
    return -1;
}

bool DBusWakeupInterface::setNetworkWakeup(const QString &logical_name, bool wake)
{
    QDBusReply<bool> reply = mp_Iface->call("setNetworkWake", logical_name, wake);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}


void DBusWakeupInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, WAKEUP_SERVICE_PATH, WAKEUP_INTERFACE, QDBusConnection::systemBus());

    // 3. create interface
    mp_InputIface = new QDBusInterface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_INTERFACE, QDBusConnection::systemBus());
}

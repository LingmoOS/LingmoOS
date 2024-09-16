// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceinterface.h"
#include "deviceinfomanager.h"
#include "mainjob.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <polkit-qt5-1/PolkitQt1/Authority>

using namespace PolkitQt1;
bool DeviceInterface::getUserAuthorPasswd()
{
#ifdef DISABLE_POLKIT
    return true;
#endif
    Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                             SystemBusNameSubject(message().service()),
                                                                             Authority::AllowUserInteraction);
    return result == Authority::Yes;
}

DeviceInterface::DeviceInterface(const char *name, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name))
            .registerObject("/org/deepin/DeviceInfo", this, opts);
}

QString DeviceInterface::getInfo(const QString &key)
{
    // 不能返回用常引用
    if ("is_server_running" != key) {
        return DeviceInfoManager::getInstance()->getInfo(key);
    }
    if (MainJob::serverIsRunning()) {
        return "1";
    }
    return "0";
}

void DeviceInterface::refreshInfo()
{
    emit sigUpdate();
}

void DeviceInterface::setMonitorDeviceFlag(bool flag)
{
    MainJob *parentMainJob = dynamic_cast<MainJob *>(parent());
    if (parentMainJob != nullptr) {
        parentMainJob->setWorkingFlag(flag);
    }
}

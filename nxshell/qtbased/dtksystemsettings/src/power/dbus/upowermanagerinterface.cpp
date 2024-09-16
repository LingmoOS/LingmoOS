// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "upowermanagerinterface.h"

#include "dpowertypes.h"

#include <qdbusconnection.h>
#include <qdbusextratypes.h>
#include <qdbuspendingreply.h>
#include <qlist.h>
#include <qstringliteral.h>

#include <QDBusObjectPath>
#include <QSharedPointer>

#include <cstddef>

DPOWER_BEGIN_NAMESPACE
UPowerManagerInterface::UPowerManagerInterface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    static const QString &Service = QStringLiteral("com.deepin.daemon.FakePower");
    static const QString &Path = QStringLiteral("/com/deepin/daemon/FakePower");
    static const QString &Interface = QStringLiteral("com.deepin.daemon.FakePower");
    QDBusConnection connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.UPower");
    const QString &Path = QStringLiteral("/org/freedesktop/UPower");
    const QString &Interface = QStringLiteral("org.freedesktop.UPower");
    QDBusConnection connection = QDBusConnection::systemBus();
    connection.connect(Service,
                       Path,
                       Interface,
                       "DeviceAdded",
                       this,
                       SIGNAL(DeviceAdded(QDBusObjectPath)));
    connection.connect(Service,
                       Path,
                       Interface,
                       "DeviceRemove",
                       this,
                       SIGNAL(DeviceRemove(QDBusObjectPath)));

#endif
    m_inter = new DDBusInterface(Service, Path, Interface, connection, this);
}

UPowerManagerInterface::~UPowerManagerInterface() { }

// properties
bool UPowerManagerInterface::lidIsClosed() const
{
    return qdbus_cast<bool>(m_inter->property("LidIsClosed"));
}

bool UPowerManagerInterface::lidIsPresent() const
{
    return qdbus_cast<bool>(m_inter->property("LidIsPresent"));
}

bool UPowerManagerInterface::onBattery() const
{
    return qdbus_cast<bool>(m_inter->property("OnBattery"));
}

QString UPowerManagerInterface::daemonVersion() const
{
    return qdbus_cast<QString>(m_inter->property("DaemonVersion"));
}

// pubilc slots
QDBusPendingReply<QList<QDBusObjectPath>> UPowerManagerInterface::enumerateDevices() const
{
    return m_inter->asyncCall(QStringLiteral("EnumerateDevices"));
}

QDBusPendingReply<QString> UPowerManagerInterface::getCriticalAction() const
{
    return m_inter->asyncCall(QStringLiteral("GetCriticalAction"));
}

QDBusPendingReply<QDBusObjectPath> UPowerManagerInterface::getDisplayDevice() const
{
    return m_inter->asyncCall(QStringLiteral("GetDisplayDevice"));
}

DPOWER_END_NAMESPACE

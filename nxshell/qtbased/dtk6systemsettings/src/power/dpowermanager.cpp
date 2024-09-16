// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dpowermanager.h"

#include "dbus/upowermanagerinterface.h"
#include "dkbdbacklight.h"
#include "dpowerdevice.h"
#include "dpowermanager_p.h"

#include <qdbusconnection.h>
#include <qdbusreply.h>
#include <qdebug.h>
#include <qsharedpointer.h>

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

void DPowerManagerPrivate::connectDBusSignal()
{
    Q_Q(DPowerManager);
    connect(m_manager_inter,
            &UPowerManagerInterface::DeviceAdded,
            q,
            [q](const QDBusObjectPath &path) {
                emit q->deviceAdded(path.path().mid(32));
            });
    connect(m_manager_inter,
            &UPowerManagerInterface::DeviceRemoved,
            q,
            [q](const QDBusObjectPath &path) {
                emit q->deviceRemoved(path.path().mid(32));
            });
    connect(m_manager_inter,
            &UPowerManagerInterface::LidIsClosedChanged,
            q,
            [q](const bool &value) {
                emit q->lidIsClosedChanged(value);
            });
    connect(m_manager_inter,
            &UPowerManagerInterface::LidIsPresentChanged,
            q,
            [q](const bool &value) {
                emit q->lidIsPresentChanged(value);
            });
}

DPowerManager::DPowerManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DPowerManagerPrivate(this))
{
    Q_D(DPowerManager);
    d->m_manager_inter = new UPowerManagerInterface(this);
    d->connectDBusSignal();
}

DPowerManager::~DPowerManager() { }

// properties
bool DPowerManager::lidIsClosed() const
{
    Q_D(const DPowerManager);
    return d->m_manager_inter->lidIsClosed();
}

bool DPowerManager::lidIsPresent() const
{
    Q_D(const DPowerManager);
    return d->m_manager_inter->lidIsPresent();
}

bool DPowerManager::hasBattery() const
{
    auto res = devices();
    if (!res)
        return false;

    for (auto const &str : res.value()) {
        if (str.contains("BAT", Qt::CaseSensitive))
            return true;
    }
    return false;
}

bool DPowerManager::onBattery() const
{
    Q_D(const DPowerManager);
    return d->m_manager_inter->onBattery();
}

bool DPowerManager::supportKbdBacklight() const
{
    const QString &service = QStringLiteral("org.freedesktop.UPower");
    const QString &path = QStringLiteral("/org/freedesktop/UPower");
    const QString &interface = QStringLiteral("org.freedesktop.DBus.Introspectable");

    QDBusMessage message = QDBusMessage::createMethodCall(service, path, interface, "Introspect");
    QDBusConnection connection = QDBusConnection::systemBus();
    QDBusReply<QString> reply = connection.call(message);
    if (!reply.isValid()) {
        qWarning() << reply.error().message();
        return false;
    }
    return reply.value().contains("KbdBacklight");
}

QString DPowerManager::daemonVersion() const
{
    Q_D(const DPowerManager);
    return d->m_manager_inter->daemonVersion();
}

// pubilc slots
DExpected<QStringList> DPowerManager::devices() const
{
    Q_D(const DPowerManager);
    QDBusPendingReply<QList<QDBusObjectPath>> reply = d->m_manager_inter->enumerateDevices();
    reply.waitForFinished();
    QStringList devices;
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    for (auto &&deviceDBus : reply.value()) {
        devices.append(deviceDBus.path().mid(32));
    }
    return devices;
}

DExpected<QString> DPowerManager::criticalAction() const
{
    Q_D(const DPowerManager);
    QDBusPendingReply<QString> reply = d->m_manager_inter->getCriticalAction();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return reply.value();
}

QSharedPointer<DPowerDevice> DPowerManager::displayDevice() const
{
    Q_D(const DPowerManager);
    QDBusPendingReply<QDBusObjectPath> reply = d->m_manager_inter->getDisplayDevice();
    reply.waitForFinished();
    if (!reply.isValid()) {
        qWarning() << reply.error().message();
        return nullptr;
    }
    QString name = reply.value().path().mid(32);
    QSharedPointer<DPowerDevice> device(new DPowerDevice(name, nullptr));
    return device;
}

QSharedPointer<DPowerDevice> DPowerManager::findDeviceByName(const QString &name) const
{
    auto res = devices();
    if (!res)
        return nullptr;

    if (!res.value().contains(name)) {
        qWarning() << QStringLiteral("Device does not exist");
        return nullptr;
    }
    QSharedPointer<DPowerDevice> device(new DPowerDevice(name, nullptr));
    return device;
}

QSharedPointer<DKbdBacklight> DPowerManager::kbdBacklight() const
{
    if (!supportKbdBacklight()) {
        qWarning() << "this devicec not support 'KbdBacklight'!";
        return nullptr;
    }
    return QSharedPointer<DKbdBacklight>(new DKbdBacklight());
}

DExpected<void> DPowerManager::refresh()
{
    auto res = devices();
    if (!res)
        return {};

    const auto &names = res.value();
    for (const auto &name : names) {
        auto device = findDeviceByName(name);
        device->refresh();
    }
    return {};
}

DPOWER_END_NAMESPACE

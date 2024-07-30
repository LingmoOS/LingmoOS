/*
    SPDX-FileCopyrightText: 2005-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DEVICEMANAGER_P_H
#define SOLID_DEVICEMANAGER_P_H

#include "managerbase_p.h"

#include "devicenotifier.h"

#include <QHash>
#include <QPointer>
#include <QSharedData>
#include <QThreadStorage>

namespace Solid
{
namespace Ifaces
{
class Device;
}
class DevicePrivate;

class DeviceManagerPrivate : public DeviceNotifier, public ManagerBasePrivate
{
    Q_OBJECT
public:
    DeviceManagerPrivate();
    ~DeviceManagerPrivate() override;

    DevicePrivate *findRegisteredDevice(const QString &udi);

private Q_SLOTS:
    void _k_deviceAdded(const QString &udi);
    void _k_deviceRemoved(const QString &udi);
    void _k_destroyed(QObject *object);

private:
    Ifaces::Device *createBackendObject(const QString &udi);

    QExplicitlySharedDataPointer<DevicePrivate> m_nullDevice;
    QHash<QString, QPointer<DevicePrivate>> m_devicesMap;
    QHash<QObject *, QString> m_reverseMap;
};

class DeviceManagerStorage
{
public:
    DeviceManagerStorage();

    QList<Ifaces::DeviceManager *> managerBackends();
    DeviceNotifier *notifier();

private:
    void ensureManagerCreated();

    QThreadStorage<DeviceManagerPrivate *> m_storage;
};
}

#endif

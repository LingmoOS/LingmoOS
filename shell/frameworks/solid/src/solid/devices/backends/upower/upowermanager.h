/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/

#ifndef UPOWERMANAGER_H
#define UPOWERMANAGER_H

#include "solid/devices/ifaces/devicemanager.h"
#include "upowerdbusinterface.h"

#include <QSet>

namespace Solid
{
namespace Backends
{
namespace UPower
{
class UPowerManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    UPowerManager(QObject *parent);
    ~UPowerManager() override;
    QObject *createDevice(const QString &udi) override;
    QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type) override;
    QStringList allDevices() override;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;
    QString udiPrefix() const override;

private Q_SLOTS:
    void onDeviceAdded(const QDBusObjectPath &path);
    void onDeviceRemoved(const QDBusObjectPath &path);

private:
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
    UPower::DBusInterface m_manager;
    QStringList m_knownDevices;
};

}
}
}
#endif // UPOWERMANAGER_H

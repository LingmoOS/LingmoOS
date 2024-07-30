/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_UDEVMANAGER_H
#define SOLID_BACKENDS_UDEV_UDEVMANAGER_H

#include <solid/devices/ifaces/devicemanager.h>

#include "../shared/udevqt.h"

namespace Solid
{
namespace Backends
{
namespace UDev
{
class UDevManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    UDevManager(QObject *parent);
    ~UDevManager() override;

    QString udiPrefix() const override;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;

    QStringList allDevices() override;

    virtual QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type) override;

    QObject *createDevice(const QString &udi) override;

private Q_SLOTS:
    void slotDeviceAdded(const UdevQt::Device &device);
    void slotDeviceRemoved(const UdevQt::Device &device);

private:
    class Private;
    Private *const d;
};
}
}
}

#endif // SOLID_BACKENDS_UDEV_UDEVMANAGER_H

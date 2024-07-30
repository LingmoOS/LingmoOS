/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_UDEVDEVICE_H
#define SOLID_BACKENDS_UDEV_UDEVDEVICE_H

#include "udev.h"

#include <QStringList>
#include <solid/devices/ifaces/device.h>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class UDevDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    UDevDevice(const UdevQt::Device device);
    ~UDevDevice() override;

    QString udi() const override;

    QString parentUdi() const override;

    QString vendor() const override;

    QString product() const override;

    QString icon() const override;

    QStringList emblems() const override;

    QString description() const override;

    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;

    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;

    QString device() const;

    QVariant property(const QString &key) const;
    QMap<QString, QVariant> allProperties() const;
    bool propertyExists(const QString &key) const;

    QString systemAttribute(const char *attribute) const;
    QString deviceName() const;
    QString devicePath() const;
    int deviceNumber() const;

    UdevQt::Device udevDevice();

private:
    UdevQt::Device m_device;
};

}
}
}
#endif // SOLID_BACKENDS_UDEV_UDEVDEVICE_H

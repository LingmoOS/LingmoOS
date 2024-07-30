/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_CAMERA_H
#define SOLID_BACKENDS_UDEV_CAMERA_H

#include "udevdeviceinterface.h"
#include <solid/devices/ifaces/camera.h>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class Camera : public DeviceInterface, virtual public Solid::Ifaces::Camera
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Camera)

public:
    Camera(UDevDevice *device);
    ~Camera() override;

    QStringList supportedProtocols() const override;
    QStringList supportedDrivers(QString protocol = QString()) const override;
    QVariant driverHandle(const QString &driver) const override;
};
}
}
}

#endif

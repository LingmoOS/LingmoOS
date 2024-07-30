/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_DEVICEINTERFACE_H
#define SOLID_BACKENDS_UDEV_DEVICEINTERFACE_H

#include "udevdevice.h"
#include <solid/devices/ifaces/deviceinterface.h>

#include <QObject>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class DeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    DeviceInterface(UDevDevice *device);
    ~DeviceInterface() override;

protected:
    UDevDevice *m_device;
};
}
}
}

#endif

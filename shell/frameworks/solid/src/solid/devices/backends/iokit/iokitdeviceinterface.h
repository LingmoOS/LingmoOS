/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_DEVICEINTERFACE_H
#define SOLID_BACKENDS_IOKIT_DEVICEINTERFACE_H

#include "iokitdevice.h"
#include <solid/devices/ifaces/deviceinterface.h>

#include <QObject>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class DeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    DeviceInterface(IOKitDevice *device);
    // the ctor taking a const device* argument makes a deep
    // copy of the IOKitDevice; any property changes made via
    // the resulting instance will not affect the original device.
    DeviceInterface(const IOKitDevice *device);
    virtual ~DeviceInterface();

protected:
    IOKitDevice *m_device;
    IOKitDevice *m_deviceCopy;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_DEVICEINTERFACE_H

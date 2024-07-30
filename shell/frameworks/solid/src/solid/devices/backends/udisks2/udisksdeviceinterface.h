/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSDEVICEINTERFACE_H
#define UDISKSDEVICEINTERFACE_H

#include "udisksdevice.h"
#include <ifaces/deviceinterface.h>

#include <QObject>
#include <QStringList>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class DeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    DeviceInterface(Device *device);
    ~DeviceInterface() override;

protected:
    Device *m_device;
};
}
}
}

#endif // UDISKSDEVICEINTERFACE_H

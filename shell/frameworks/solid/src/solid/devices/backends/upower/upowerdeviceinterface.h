/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UPOWERDEVICEINTERFACE_H
#define UPOWERDEVICEINTERFACE_H

#include "upowerdevice.h"
#include <ifaces/deviceinterface.h>

#include <QObject>
#include <QPointer>
#include <QStringList>

namespace Solid
{
namespace Backends
{
namespace UPower
{
class DeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    DeviceInterface(UPowerDevice *device);
    ~DeviceInterface() override;

protected:
    QPointer<UPowerDevice> m_device;
};
}
}
}

#endif // UPOWERDEVICEINTERFACE_H

/*
    SPDX-FileCopyrightText: 2020 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IMOBILE_IMOBILEDEVICEINTERFACE_H
#define SOLID_BACKENDS_IMOBILE_IMOBILEDEVICEINTERFACE_H

#include <solid/devices/ifaces/deviceinterface.h>
//#include "udevdevice.h"

#include <QObject>

namespace Solid
{
namespace Backends
{
namespace IMobile
{
class IMobileDevice;

class DeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)

public:
    explicit DeviceInterface(IMobileDevice *device);
    ~DeviceInterface() override;

protected:
    IMobileDevice *m_device;
};

}
}
}

#endif // SOLID_BACKENDS_IMOBILE_IMOBILEDEVICEINTERFACE_H

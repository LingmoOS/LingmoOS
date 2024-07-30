/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef SOLID_BACKENDS_FAKEHW_FAKEDEVICEINTERFACE_H
#define SOLID_BACKENDS_FAKEHW_FAKEDEVICEINTERFACE_H

#include <QObject>
#include <solid/devices/ifaces/deviceinterface.h>

#include "fakedevice.h"

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    explicit FakeDeviceInterface(FakeDevice *device);
    ~FakeDeviceInterface() override;

protected:
    FakeDevice *fakeDevice() const
    {
        return m_device;
    }
    FakeDevice *fakeDevice()
    {
        return m_device;
    }

private:
    FakeDevice *m_device;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEDEVICEINTERFACE_H

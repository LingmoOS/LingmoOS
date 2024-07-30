/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKECAMERA_H
#define SOLID_BACKENDS_FAKEHW_FAKECAMERA_H

#include "fakedeviceinterface.h"
#include <solid/devices/ifaces/camera.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeCamera : public FakeDeviceInterface, virtual public Solid::Ifaces::Camera
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Camera)

public:
    explicit FakeCamera(FakeDevice *device);
    ~FakeCamera() override;

public Q_SLOTS:
    QStringList supportedProtocols() const override;
    QStringList supportedDrivers(QString protocol) const override;
    QVariant driverHandle(const QString &driver) const override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKECAMERA_H

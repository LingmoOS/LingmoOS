/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>
    SPDX-FileCopyrightText: 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEPORTABLEMEDIAPLAYER_H
#define SOLID_BACKENDS_FAKEHW_FAKEPORTABLEMEDIAPLAYER_H

#include "fakedeviceinterface.h"
#include <solid/devices/ifaces/portablemediaplayer.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakePortableMediaPlayer : public FakeDeviceInterface, virtual public Solid::Ifaces::PortableMediaPlayer
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::PortableMediaPlayer)

public:
    explicit FakePortableMediaPlayer(FakeDevice *device);
    ~FakePortableMediaPlayer() override;

public Q_SLOTS:
    QStringList supportedProtocols() const override;
    QStringList supportedDrivers(QString protocol) const override;
    QVariant driverHandle(const QString &driver) const override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEPORTABLEMEDIAPLAYER_H

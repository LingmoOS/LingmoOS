/*
    SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKE_NETWORKSHARE_H
#define SOLID_BACKENDS_FAKE_NETWORKSHARE_H

#include "fakedeviceinterface.h"
#include <solid/devices/ifaces/networkshare.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeNetworkShare : public FakeDeviceInterface, public Solid::Ifaces::NetworkShare
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::NetworkShare)

public:
    explicit FakeNetworkShare(FakeDevice *device);

    ~FakeNetworkShare() override;

    Solid::NetworkShare::ShareType type() const override;

    QUrl url() const override;
};

}
}
}

#endif // SOLID_BACKENDS_FAKE_NETWORKSHARE_H

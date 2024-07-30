/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEBLOCK_H
#define SOLID_BACKENDS_FAKEHW_FAKEBLOCK_H

#include "fakedeviceinterface.h"
#include <solid/devices/ifaces/block.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeBlock : public FakeDeviceInterface, virtual public Solid::Ifaces::Block
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Block)

public:
    explicit FakeBlock(FakeDevice *device);
    ~FakeBlock() override;

public Q_SLOTS:
    int deviceMajor() const override;
    int deviceMinor() const override;
    QString device() const override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEBLOCK_H

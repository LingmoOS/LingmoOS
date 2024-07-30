/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_BLOCK_H
#define SOLID_BACKENDS_IOKIT_BLOCK_H

#include "iokitdeviceinterface.h"
#include <solid/devices/ifaces/block.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class Block : public DeviceInterface, virtual public Solid::Ifaces::Block
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Block)

public:
    Block(IOKitDevice *device);
    Block(const IOKitDevice *device);
    virtual ~Block();

    int deviceMajor() const override;
    int deviceMinor() const override;
    QString device() const override;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_BLOCK_H

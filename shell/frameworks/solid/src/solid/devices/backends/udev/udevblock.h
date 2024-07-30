/*
    SPDX-FileCopyrightText: 2010 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_UDEVBLOCK_H
#define SOLID_BACKENDS_UDEV_UDEVBLOCK_H

#include <solid/devices/ifaces/block.h>

#include "udevdeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace UDev
{
class Block : public DeviceInterface, virtual public Solid::Ifaces::Block
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Block)

public:
    Block(UDevDevice *device);
    ~Block() override;

    int deviceMajor() const override;
    int deviceMinor() const override;
    QString device() const override;
};
}
}
}

#endif // SOLID_BACKENDS_UDEV_UDEVBLOCK_H

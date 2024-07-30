/*
    SPDX-FileCopyrightText: 2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSBLOCK_H
#define UDISKSBLOCK_H

#include "udisksdeviceinterface.h"
#include <solid/devices/ifaces/block.h>

#include <sys/types.h> // dev_t

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class Block : public DeviceInterface, virtual public Solid::Ifaces::Block
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Block)

public:
    Block(Device *dev);
    ~Block() override;

    QString device() const override;
    int deviceMinor() const override;
    int deviceMajor() const override;

private:
    dev_t m_devNum;
    QString m_devFile;
};

}
}
}

#endif // UDISKSBLOCK_H

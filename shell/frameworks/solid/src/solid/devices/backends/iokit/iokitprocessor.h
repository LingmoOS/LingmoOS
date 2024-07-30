/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_PROCESSOR_H
#define SOLID_BACKENDS_IOKIT_PROCESSOR_H

#include "iokitdeviceinterface.h"
#include <solid/devices/ifaces/processor.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitDevice;

class Processor : public DeviceInterface, virtual public Solid::Ifaces::Processor
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Processor)

public:
    Processor(IOKitDevice *device);
    virtual ~Processor();

    int number() const override;
    int maxSpeed() const override;
    bool canChangeFrequency() const override;
    Solid::Processor::InstructionSets instructionSets() const override;
    static QString vendor();
    static QString product();
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_PROCESSOR_H

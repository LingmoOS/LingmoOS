/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_PROCESSOR_H
#define SOLID_BACKENDS_UDEV_PROCESSOR_H

#include "udevdeviceinterface.h"
#include <solid/devices/ifaces/processor.h>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class UDevDevice;

class Processor : public DeviceInterface, virtual public Solid::Ifaces::Processor
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Processor)

public:
    Processor(UDevDevice *device);
    ~Processor() override;

    int number() const override;
    int maxSpeed() const override;
    bool canChangeFrequency() const override;
    Solid::Processor::InstructionSets instructionSets() const override;

private:
    enum CanChangeFrequencyEnum {
        NotChecked,
        CanChangeFreq,
        CannotChangeFreq,
    };
    mutable CanChangeFrequencyEnum m_canChangeFrequency;
    mutable int m_maxSpeed;
    QString prefix() const;
};
}
}
}

#endif // SOLID_BACKENDS_UDEV_PROCESSOR_H

/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_PROCESSOR_H
#define SOLID_IFACES_PROCESSOR_H

#include <solid/devices/ifaces/deviceinterface.h>
#include <solid/processor.h>

namespace Solid
{
namespace Ifaces
{
/**
 * This device interface is available on processors.
 */
class Processor : virtual public DeviceInterface
{
public:
    /**
     * Destroys a Processor object.
     */
    ~Processor() override;

    /**
     * Retrieves the processor number in the system.
     *
     * @return the internal processor number in the system, starting from zero
     */
    virtual int number() const = 0;

    /**
     * Retrieves the maximum speed of the processor.
     *
     * @return the maximum speed in MHz
     */
    virtual int maxSpeed() const = 0;

    /**
     * Indicates if the processor can change the CPU frequency.
     *
     * True if a processor is able to change its own CPU frequency.
     *  (generally for power management).
     *
     * @return true if the processor can change CPU frequency, false otherwise
     */
    virtual bool canChangeFrequency() const = 0;

    /**
     * Queries the instructions set extensions of the CPU.
     *
     * @return the extensions supported by the CPU
     */
    virtual Solid::Processor::InstructionSets instructionSets() const = 0;
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Processor, "org.kde.Solid.Ifaces.Processor/0.1")

#endif

/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEPROCESSOR_H
#define SOLID_BACKENDS_FAKEHW_FAKEPROCESSOR_H

#include "fakedeviceinterface.h"
#include <solid/devices/ifaces/processor.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeProcessor : public FakeDeviceInterface, public Solid::Ifaces::Processor
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Processor)

public:
    explicit FakeProcessor(FakeDevice *device);
    ~FakeProcessor() override;

public Q_SLOTS:
    int number() const override;
    int maxSpeed() const override;
    bool canChangeFrequency() const override;
    Solid::Processor::InstructionSets instructionSets() const override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEPROCESSOR_H

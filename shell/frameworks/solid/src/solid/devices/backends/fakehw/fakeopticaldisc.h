/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEOPTICALDISC_H
#define SOLID_BACKENDS_FAKEHW_FAKEOPTICALDISC_H

#include "fakevolume.h"
#include <solid/devices/ifaces/opticaldisc.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeOpticalDisc : public FakeVolume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    explicit FakeOpticalDisc(FakeDevice *device);
    ~FakeOpticalDisc() override;

public Q_SLOTS:
    Solid::OpticalDisc::ContentTypes availableContent() const override;
    Solid::OpticalDisc::DiscType discType() const override;
    bool isAppendable() const override;
    bool isBlank() const override;
    bool isRewritable() const override;
    qulonglong capacity() const override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEOPTICALDISC_H

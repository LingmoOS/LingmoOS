/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKECDROM_H
#define SOLID_BACKENDS_FAKEHW_FAKECDROM_H

#include "fakestorage.h"
#include <solid/devices/ifaces/opticaldrive.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeCdrom : public FakeStorage, virtual public Solid::Ifaces::OpticalDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDrive)

public:
    explicit FakeCdrom(FakeDevice *device);
    ~FakeCdrom() override;

public Q_SLOTS:
    Solid::OpticalDrive::MediumTypes supportedMedia() const override;
    int readSpeed() const override;
    int writeSpeed() const override;
    QList<int> writeSpeeds() const override;
    bool eject() override;

Q_SIGNALS:
    void ejectPressed(const QString &udi) override;
    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKECDROM_H

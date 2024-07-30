/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEVOLUME_H
#define SOLID_BACKENDS_FAKEHW_FAKEVOLUME_H

#include "fakeblock.h"
#include <solid/devices/ifaces/storagevolume.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeVolume : public FakeBlock, virtual public Solid::Ifaces::StorageVolume
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageVolume)

public:
    explicit FakeVolume(FakeDevice *device);
    ~FakeVolume() override;

public Q_SLOTS:
    bool isIgnored() const override;
    Solid::StorageVolume::UsageType usage() const override;
    QString fsType() const override;
    QString label() const override;
    QString uuid() const override;
    qulonglong size() const override;
    QString encryptedContainerUdi() const override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEVOLUME_H

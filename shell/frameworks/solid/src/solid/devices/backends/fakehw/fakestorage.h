/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKESTORAGE_H
#define SOLID_BACKENDS_FAKEHW_FAKESTORAGE_H

#include "fakeblock.h"
#include <solid/devices/ifaces/storagedrive.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeStorage : public FakeBlock, virtual public Solid::Ifaces::StorageDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageDrive)

public:
    explicit FakeStorage(FakeDevice *device);
    ~FakeStorage() override;

public Q_SLOTS:
    Solid::StorageDrive::Bus bus() const override;
    Solid::StorageDrive::DriveType driveType() const override;

    bool isRemovable() const override;
    bool isHotpluggable() const override;
    qulonglong size() const override;

    QDateTime timeDetected() const override;
    QDateTime timeMediaDetected() const override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKESTORAGE_H

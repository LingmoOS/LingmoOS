/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_IOKITSTORAGE_H
#define SOLID_BACKENDS_IOKIT_IOKITSTORAGE_H

#include "dadictionary_p.h"
#include "iokitblock.h"

#include <solid/devices/ifaces/storagedrive.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitStorage : public Block, virtual public Solid::Ifaces::StorageDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageDrive)

public:
    explicit IOKitStorage(IOKitDevice *device);
    explicit IOKitStorage(const IOKitDevice *device);
    ~IOKitStorage();

    QString vendor() const;
    QString product() const;
    QString description() const;

public Q_SLOTS:
    Solid::StorageDrive::Bus bus() const override;
    Solid::StorageDrive::DriveType driveType() const override;

    bool isRemovable() const override;
    bool isHotpluggable() const override;
    qulonglong size() const override;

private:
    DADictionary *daDict;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_IOKITSTORAGE_H

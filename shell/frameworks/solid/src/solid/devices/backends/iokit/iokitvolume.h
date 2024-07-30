/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_VOLUME_H
#define SOLID_BACKENDS_IOKIT_VOLUME_H

#include "dadictionary_p.h"
#include "iokitblock.h"
#include <solid/devices/ifaces/storagevolume.h>

#include <DiskArbitration/DiskArbitration.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitVolume : public Block, virtual public Solid::Ifaces::StorageVolume
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageVolume)

public:
    IOKitVolume(IOKitDevice *device);
    IOKitVolume(const IOKitDevice *device);
    virtual ~IOKitVolume();

    bool isIgnored() const override;
    Solid::StorageVolume::UsageType usage() const override;
    QString fsType() const override;
    QString label() const override;
    QString uuid() const override;
    qulonglong size() const override;
    QString encryptedContainerUdi() const override;

    QString vendor() const;
    QString product() const;
    QString description() const;

    DADiskRef daRef() const;

private:
    DADictionary *daDict;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_VOLUME_H

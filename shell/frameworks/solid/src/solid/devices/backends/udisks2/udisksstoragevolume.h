/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSSTORAGEVOLUME_H
#define UDISKSSTORAGEVOLUME_H

#include "udisksblock.h"
#include <ifaces/storagevolume.h>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class StorageVolume : public Block, virtual public Solid::Ifaces::StorageVolume
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageVolume)

public:
    StorageVolume(Device *device);
    ~StorageVolume() override;

    QString encryptedContainerUdi() const override;
    qulonglong size() const override;
    QString uuid() const override;
    QString label() const override;
    QString fsType() const override;
    Solid::StorageVolume::UsageType usage() const override;
    bool isIgnored() const override;
};

}
}
}

#endif // UDISKS2STORAGEVOLUME_H

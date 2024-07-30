/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINSTORAGEVOLUME_H
#define WINSTORAGEVOLUME_H

#include <solid/devices/ifaces/storagevolume.h>

#include "winblock.h"
#include "wininterface.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinStorageVolume : public WinBlock, public virtual Solid::Ifaces::StorageVolume
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageVolume)

public:
    WinStorageVolume(WinDevice *device);
    ~WinStorageVolume();

    virtual bool isIgnored() const;

    virtual Solid::StorageVolume::UsageType usage() const;
    virtual QString fsType() const;
    virtual QString label() const;
    virtual QString uuid() const;
    virtual qulonglong size() const;
    virtual QString encryptedContainerUdi() const;

private:
    QString m_fs;
    QString m_label;
    QString m_uuid;
    qulonglong m_size;

    void updateCache();
};

}
}
}

#endif // WINVOLUME_H

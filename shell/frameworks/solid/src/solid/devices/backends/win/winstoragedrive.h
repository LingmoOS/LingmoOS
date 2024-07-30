/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINSTORAGEDRIVE_H
#define WINSTORAGEDRIVE_H

#include <solid/devices/ifaces/storagedrive.h>

#include "winblock.h"
#include "windevice.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinStorageDrive : public WinBlock, public virtual Solid::Ifaces::StorageDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageDrive)
public:
    WinStorageDrive(WinDevice *device);
    ~WinStorageDrive();

    virtual Solid::StorageDrive::Bus bus() const;

    virtual Solid::StorageDrive::DriveType driveType() const;

    virtual bool isRemovable() const;

    virtual bool isHotpluggable() const;

    virtual qulonglong size() const;

private:
    void updateCache();

    Solid::StorageDrive::Bus m_bus;
    qulonglong m_size;
    bool m_isHotplugges;
    bool m_isRemovable;
};
}
}
}

#endif // WINSTORAGEDRIVE_H

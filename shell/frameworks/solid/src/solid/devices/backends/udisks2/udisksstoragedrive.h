/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSSTORAGEDRIVE_H
#define UDISKSSTORAGEDRIVE_H

#include <QDateTime>
#include <ifaces/storagedrive.h>

#include "../shared/udevqt.h"
#include <config-solid.h>

#include "udisksblock.h"

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class StorageDrive : public Block, virtual public Solid::Ifaces::StorageDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageDrive)

public:
    StorageDrive(Device *dev);
    ~StorageDrive() override;

    qulonglong size() const override;
    bool isHotpluggable() const override;
    bool isRemovable() const override;
    Solid::StorageDrive::DriveType driveType() const override;
    Solid::StorageDrive::Bus bus() const override;
    QDateTime timeDetected() const override;
    QDateTime timeMediaDetected() const override;

private:
#if UDEV_FOUND
    UdevQt::Device m_udevDevice;
#endif
};

}
}
}

#endif // UDISKSSTORAGEDRIVE_H

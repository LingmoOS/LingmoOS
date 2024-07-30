/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINSTORAGEACCESS_H
#define WINSTORAGEACCESS_H

#include <solid/devices/ifaces/storageaccess.h>

#include "windevice.h"
#include "wininterface.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinStorageAccess : public WinInterface, public virtual Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)
public:
    WinStorageAccess(WinDevice *device);
    ~WinStorageAccess();

    virtual bool isAccessible() const;

    virtual QString filePath() const;

    virtual bool isIgnored() const;

    virtual bool isEncrypted() const;

    virtual bool setup();

    virtual bool teardown();

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi);

    void setupDone(Solid::ErrorType error, QVariant resultData, const QString &udi);

    void teardownDone(Solid::ErrorType error, QVariant resultData, const QString &udi);

    void setupRequested(const QString &udi);

    void teardownRequested(const QString &udi);
};
}
}
}

#endif // WINSTORAGEACCESS_H

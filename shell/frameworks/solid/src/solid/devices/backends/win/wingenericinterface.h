/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINGENERICINTERFACE_H
#define WINGENERICINTERFACE_H

#include "wininterface.h"
#include <solid/devices/ifaces/genericinterface.h>
namespace Solid
{
namespace Backends
{
namespace Win
{
class WinGenericInterface : public WinInterface, virtual public Solid::Ifaces::GenericInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::GenericInterface)
public:
    WinGenericInterface(WinDevice *device);

    virtual QVariant property(const QString &key) const;

    virtual QMap<QString, QVariant> allProperties() const;

    virtual bool propertyExists(const QString &key) const;

Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);
};
}
}
}

#endif // WINGENERICINTERFACE_H

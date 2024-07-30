/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_GENERICINTERFACE_H
#define SOLID_BACKENDS_IOKIT_GENERICINTERFACE_H

#include "iokitdeviceinterface.h"
#include <solid/devices/ifaces/genericinterface.h>
#include <solid/genericinterface.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitDevice;

class GenericInterface : public DeviceInterface, virtual public Solid::Ifaces::GenericInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::GenericInterface)

public:
    GenericInterface(IOKitDevice *device);
    virtual ~GenericInterface();

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

#endif // SOLID_BACKENDS_IOKIT_GENERICINTERFACE_H

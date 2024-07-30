/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDEV_GENERICINTERFACE_H
#define SOLID_BACKENDS_UDEV_GENERICINTERFACE_H

#include "udevdeviceinterface.h"
#include <solid/devices/ifaces/genericinterface.h>
#include <solid/genericinterface.h>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class UDevDevice;

class GenericInterface : public DeviceInterface, virtual public Solid::Ifaces::GenericInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::GenericInterface)

public:
    GenericInterface(UDevDevice *device);
    ~GenericInterface() override;

    QVariant property(const QString &key) const override;
    QMap<QString, QVariant> allProperties() const override;
    bool propertyExists(const QString &key) const override;

Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changes) override;
    void conditionRaised(const QString &condition, const QString &reason) override;
};
}
}
}

#endif

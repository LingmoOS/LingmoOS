/*
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEGENERICINTERFACE_H
#define SOLID_BACKENDS_FAKEHW_FAKEGENERICINTERFACE_H

#include "fakedeviceinterface.h"
#include <solid/devices/ifaces/genericinterface.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeGenericInterface : public FakeDeviceInterface, public Solid::Ifaces::GenericInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::GenericInterface)

public:
    explicit FakeGenericInterface(FakeDevice *device);
    ~FakeGenericInterface() override;

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

#endif // SOLID_BACKENDS_FAKEHW_FAKEGENERICINTERFACE_H

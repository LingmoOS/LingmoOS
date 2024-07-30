/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_SHARED_ROOT_DEVICE_H
#define SOLID_BACKENDS_SHARED_ROOT_DEVICE_H

#include <solid/devices/ifaces/device.h>

#include <QStringList>

namespace Solid
{
namespace Backends
{
namespace Shared
{
class RootDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    explicit RootDevice(const QString &udi, const QString &parentUdi = QString());

    ~RootDevice() override;

    QString udi() const override;
    QString parentUdi() const override;

    QString vendor() const override;
    void setVendor(const QString &vendor);

    QString product() const override;
    void setProduct(const QString &product);

    QString icon() const override;
    void setIcon(const QString &icon);

    QStringList emblems() const override;
    void setEmblems(const QStringList &emblems);

    QString description() const override;
    void setDescription(const QString &description);

    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;

    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;

private:
    QString m_udi;
    QString m_parentUdi;
    QString m_vendor;
    QString m_product;
    QString m_icon;
    QStringList m_emblems;
    QString m_description;
};

}
}
}
#endif

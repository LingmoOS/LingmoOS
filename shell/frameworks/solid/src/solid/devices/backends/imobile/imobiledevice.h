/*
    SPDX-FileCopyrightText: 2020 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IMOBILE_IMOBILEDEVICE_H
#define SOLID_BACKENDS_IMOBILE_IMOBILEDEVICE_H

#include <QStringList>
#include <solid/devices/ifaces/device.h>

namespace Solid
{
namespace Backends
{
namespace IMobile
{
class IMobileDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    explicit IMobileDevice(const QString &udi);
    ~IMobileDevice() override;

    QString udi() const override;
    QString parentUdi() const override;

    QString vendor() const override;
    QString product() const override;
    QString icon() const override;
    QStringList emblems() const override;
    QString description() const override;

    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;

    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;

private:
    QString m_udi;

    QString m_name;
    QString m_deviceClass;
};

} // namespace IMobile
} // namespace Backends
} // namespace Solid

#endif // SOLID_BACKENDS_IMOBILE_IMOBILEDEVICE_H

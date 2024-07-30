/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FSTAB_FSTABMANAGER_H
#define SOLID_BACKENDS_FSTAB_FSTABMANAGER_H

#include <QSet>
#include <QStringList>
#include <solid/deviceinterface.h>
#include <solid/devices/ifaces/devicemanager.h>

namespace Solid
{
namespace Backends
{
namespace Fstab
{
class AbstractDeviceFactory;

class FstabManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    explicit FstabManager(QObject *parent);
    ~FstabManager() override;

    QString udiPrefix() const override;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;
    QStringList allDevices() override;
    QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type) override;
    QObject *createDevice(const QString &udi) override;

Q_SIGNALS:
    void mtabChanged(const QString &device);

private Q_SLOTS:
    void onFstabChanged();
    void onMtabChanged();

private:
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
    QStringList m_deviceList;
    void _k_updateDeviceList();
};

}
}
}

#endif

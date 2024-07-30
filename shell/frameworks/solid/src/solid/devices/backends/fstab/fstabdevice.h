/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FSTAB_FSTAB_DEVICE_H
#define SOLID_BACKENDS_FSTAB_FSTAB_DEVICE_H

#include "fstabstorageaccess.h"
#include <QPointer>
#include <QStringList>
#include <solid/devices/ifaces/device.h>

namespace Solid
{
namespace Backends
{
namespace Fstab
{
class FstabDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    FstabDevice(QString uid);

    ~FstabDevice() override;

    QString udi() const override;

    QString parentUdi() const override;

    QString vendor() const override;

    QString product() const override;

    QString icon() const override;

    QStringList emblems() const override;

    QString displayName() const override;

    QString description() const override;

    bool isEncrypted() const;

    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;

    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;

    QString device() const;

Q_SIGNALS:
    void mtabChanged(const QString &device);

public Q_SLOTS:
    void onMtabChanged(const QString &device);

private:
    QString m_uid;
    QString m_device;
    QString m_product;
    QString m_vendor;
    QString m_displayName;
    QString m_description;
    QString m_iconName;
    QPointer<FstabStorageAccess> m_storageAccess;
    enum class StorageType : uint8_t {
        Other = 0,
        NetworkShare,
        Encrypted,
    };
    StorageType m_storageType = StorageType::Other;
};

}
}
}
#endif // SOLID_BACKENDS_FSTAB_FSTAB_DEVICE_H

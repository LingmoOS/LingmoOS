/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSDEVICE_H
#define UDISKSDEVICE_H

#include "udisks2.h"

#include <ifaces/device.h>
#include <solid/deviceinterface.h>
#include <solid/solidnamespace.h>

#include <QDBusObjectPath>
#include <QStringList>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class DeviceBackend;

class Device : public Solid::Ifaces::Device
{
    Q_OBJECT
public:
    Device(const QString &udi);
    ~Device() override;

    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;
    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;
    QString description() const override;
    QStringList emblems() const override;
    QString icon() const override;
    QString product() const override;
    QString vendor() const override;
    QString udi() const override;
    QString parentUdi() const override;

    QVariant prop(const QString &key) const;
    bool propertyExists(const QString &key) const;
    QVariantMap allProperties() const;
    void invalidateCache();

    bool hasInterface(const QString &name) const;
    QStringList interfaces() const;

    QString errorToString(const QString &error) const;
    Solid::ErrorType errorToSolidError(const QString &error) const;

    bool isBlock() const;
    bool isPartition() const;
    bool isPartitionTable() const;
    bool isStorageVolume() const;
    bool isStorageAccess() const;
    bool isDrive() const;
    bool isOpticalDrive() const;
    bool isOpticalDisc() const;
    bool mightBeOpticalDisc() const;
    bool isMounted() const;
    bool isEncryptedContainer() const;
    bool isEncryptedCleartext() const;
    bool isRoot() const;
    bool isSwap() const;
    bool isLoop() const;

    QString drivePath() const;

Q_SIGNALS:
    void changed();
    void propertyChanged(const QMap<QString, int> &changes);

protected:
    QPointer<DeviceBackend> m_backend;

private:
    QString loopDescription() const;
    QString storageDescription() const;
    QString volumeDescription() const;
};

}
}
}

#endif // UDISKSDEVICE_H

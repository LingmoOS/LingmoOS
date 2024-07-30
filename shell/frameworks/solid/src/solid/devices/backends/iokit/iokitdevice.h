/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_IOKITDEVICE_H
#define SOLID_BACKENDS_IOKIT_IOKITDEVICE_H

#include <IOKit/IOKitLib.h>
#include <solid/devices/ifaces/device.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitDevicePrivate;
class IOKitManager;

class IOKitDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    IOKitDevice(const QString &udi);
    IOKitDevice(const IOKitDevice &device);
    virtual ~IOKitDevice();

    QString udi() const override;
    QString parentUdi() const override;

    QString vendor() const override;
    QString product() const override;
    QString icon() const override;
    QStringList emblems() const override;
    QString description() const override;

    virtual QVariant property(const QString &key) const;

    virtual QMap<QString, QVariant> allProperties() const;

    virtual bool iOKitPropertyExists(const QString &key) const;

    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;
    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;

    bool conformsToIOKitClass(const QString &className) const;

Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);

private:
    friend class IOKitManager;
    IOKitDevice(const QString &udi, const io_registry_entry_t &entry);
    IOKitDevicePrivate *const d;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_IOKITDEVICE_H

/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_IOKITMANAGER_H
#define SOLID_BACKENDS_IOKIT_IOKITMANAGER_H

#include <solid/deviceinterface.h>
#include <solid/devices/ifaces/devicemanager.h>

#include <QStringList>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitManagerPrivate;

class IOKitManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    IOKitManager(QObject *parent);
    virtual ~IOKitManager();

    virtual QString udiPrefix() const;
    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    virtual QStringList allDevices();
    virtual QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type);
    virtual QObject *createDevice(const QString &udi);

private:
    IOKitManagerPrivate *d;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_IOKITMANAGER_H

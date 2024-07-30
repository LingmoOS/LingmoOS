/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSMANAGER_H
#define UDISKSMANAGER_H

#include "dbus/manager.h"
#include "udisks2.h"
#include "udisksdevice.h"

#include <solid/devices/ifaces/devicemanager.h>

#include <QSet>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class Manager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    Manager(QObject *parent);
    QObject *createDevice(const QString &udi) override;
    QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type) override;
    QStringList allDevices() override;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;
    QString udiPrefix() const override;
    ~Manager() override;

private Q_SLOTS:
    void slotInterfacesAdded(const QDBusObjectPath &object_path, const VariantMapMap &interfaces_and_properties);
    void slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void slotMediaChanged(const QDBusMessage &msg);

private:
    const QStringList &deviceCache();
    void introspect(const QString &path, bool checkOptical = false);
    void updateBackend(const QString &udi);
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
    org::freedesktop::DBus::ObjectManager m_manager;
    QStringList m_deviceCache;
};

}
}
}
#endif // UDISKSMANAGER_H

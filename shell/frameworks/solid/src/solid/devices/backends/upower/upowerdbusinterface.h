/*
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UPOWER_DBUSINTERFACE_H
#define SOLID_BACKENDS_UPOWER_DBUSINTERFACE_H

#include <QDBusConnectionInterface>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QList>
#include <QObject>

namespace Solid
{
namespace Backends
{
namespace UPower
{
/*
 * Proxy class for interface org.freedesktop.UPower
 */
class DBusInterface: public QDBusAbstractInterface
{
    Q_OBJECT

public:
    DBusInterface(const QDBusConnection &connection, QObject *parent = nullptr);

    ~DBusInterface();

public Q_SLOTS:
    QDBusPendingReply<QList<QDBusObjectPath>> EnumerateDevices();

Q_SIGNALS:
    void DeviceAdded(const QDBusObjectPath &device);
    void DeviceRemoved(const QDBusObjectPath &device);
};

}
}
}
#endif // SOLID_BACKENDS_UPOWER_DBUSINTERFACE_H

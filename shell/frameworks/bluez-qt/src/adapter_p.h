/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_ADAPTER_P_H
#define BLUEZQT_ADAPTER_P_H

#include <QObject>
#include <QStringList>

#include "bluezadapter1.h"
#include "bluezqt_dbustypes.h"
#include "dbusproperties.h"
#include "types.h"

namespace BluezQt
{
typedef org::bluez::Adapter1 BluezAdapter;
typedef org::freedesktop::DBus::Properties DBusProperties;

class AdapterPrivate : public QObject
{
    Q_OBJECT

public:
    explicit AdapterPrivate(const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);

    void interfacesAdded(const QString &path, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QString &path, const QStringList &interfaces);

    void addDevice(const DevicePtr &device);
    void removeDevice(const DevicePtr &device);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);
    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    QWeakPointer<Adapter> q;
    BluezAdapter *m_bluezAdapter;
    DBusProperties *m_dbusProperties;

    QString m_address;
    QString m_name;
    QString m_alias;
    quint32 m_adapterClass;
    bool m_powered;
    bool m_discoverable;
    quint32 m_discoverableTimeout;
    bool m_pairable;
    quint32 m_pairableTimeout;
    bool m_discovering;
    QStringList m_uuids;
    QList<DevicePtr> m_devices;
    QString m_modalias;
    MediaPtr m_media;
    GattManagerPtr m_gattManager;
    LEAdvertisingManagerPtr m_leAdvertisingManager;
};

} // namespace BluezQt

#endif // BLUEZQT_ADAPTER_P_H

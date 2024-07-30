/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTSERVICEREMOTE_P_H
#define BLUEZQT_GATTSERVICEREMOTE_P_H

#include <QObject>
#include <QStringList>

#include "types.h"
#include "bluezgattservice1.h"
#include "dbusproperties.h"
#include "bluezqt_dbustypes.h"

namespace BluezQt
{

typedef org::bluez::GattService1 BluezGattService;
typedef org::freedesktop::DBus::Properties DBusProperties;

class GattServiceRemotePrivate : public QObject
{
    Q_OBJECT

public:
    explicit GattServiceRemotePrivate(const QString &path, const QVariantMap &properties, const DevicePtr &device);

    void init(const QVariantMap &properties);

    void interfacesAdded(const QString &path, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QString &path, const QStringList &interfaces);

    void addGattCharacteristic(const QString &gattCharacteristicPath, const QVariantMap &properties);
    void removeGattCharacteristic(const QString &gattCharacteristicPath);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);
    void propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    QWeakPointer<GattServiceRemote> q;
    BluezGattService *m_bluezGattService;
    DBusProperties *m_dbusProperties;

    QString m_uuid;
    bool m_primary;
    const DevicePtr m_device;
    QList<QDBusObjectPath> m_includes;
    quint16 m_handle;
    QList<GattCharacteristicRemotePtr> m_characteristics;
};

} // namespace BluezQt

#endif // BLUEZQT_GATTSERVICEREMOTE_P_H

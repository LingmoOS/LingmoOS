/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTCHARACTERISTICREMOTE_P_H
#define BLUEZQT_GATTCHARACTERISTICREMOTE_P_H

#include <QObject>
#include <QStringList>

#include "types.h"
#include "bluezgattcharacteristic1.h"
#include "dbusproperties.h"
#include "bluezqt_dbustypes.h"

namespace BluezQt
{

typedef org::bluez::GattCharacteristic1 BluezGattCharacteristic;
typedef org::freedesktop::DBus::Properties DBusProperties;

class GattCharacteristicRemotePrivate : public QObject
{
    Q_OBJECT

public:
    explicit GattCharacteristicRemotePrivate(const QString &path, const QVariantMap &properties, const GattServiceRemotePtr &service);

    void init(const QVariantMap &properties);

    void interfacesAdded(const QString &path, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QString &path, const QStringList &interfaces);

    void addGattDescriptor(const QString &gattDescriptorPath, const QVariantMap &properties);
    void removeGattDescriptor(const QString &gattDescriptorPath);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);
    void propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    QWeakPointer<GattCharacteristicRemote> q;
    BluezGattCharacteristic *m_bluezGattCharacteristic;
    DBusProperties *m_dbusProperties;

    QString m_uuid;
    QByteArray m_value;
    bool m_writeAcquired;
    bool m_notifyAcquired;
    bool m_notifying;
    QStringList m_flags;
    quint16 m_handle;
    quint16 m_MTU;
    const GattServiceRemotePtr m_service;
    QList<GattDescriptorRemotePtr> m_descriptors;
};

} // namespace BluezQt

#endif // BLUEZQT_GATTCHARACTERISTIC_P_H

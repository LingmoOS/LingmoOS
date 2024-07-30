/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTDESCRIPTORREMOTE_P_H
#define BLUEZQT_GATTDESCRIPTORREMOTE_P_H

#include <QObject>
#include <QMap>

#include "types.h"
#include "bluezgattdescriptor1.h"
#include "dbusproperties.h"
#include "bluezqt_dbustypes.h"

namespace BluezQt
{

typedef org::bluez::GattDescriptor1 BluezGattDescriptor;
typedef org::freedesktop::DBus::Properties DBusProperties;


class GattDescriptorRemotePrivate : public QObject
{
    Q_OBJECT

public:
    explicit GattDescriptorRemotePrivate(const QString &path, const QVariantMap &properties, const GattCharacteristicRemotePtr &characteristic);

    void init(const QVariantMap &properties);

    QDBusPendingReply<> setDBusProperty(const QString &name, const QVariant &value);
    void propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    QWeakPointer<GattDescriptorRemote> q;
    BluezGattDescriptor *m_bluezGattDescriptor;
    DBusProperties *m_dbusProperties;

    QString m_uuid;
    QByteArray m_value;
    QStringList m_flags;
    quint16 m_handle;
    const GattCharacteristicRemotePtr m_characteristic;
};

} // namespace BluezQt

#endif // BLUEZQT_GATTDESCRIPTORREMOTE_P_H

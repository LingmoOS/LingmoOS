/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTCHARACTERISTIC_P_H
#define BLUEZQT_GATTCHARACTERISTIC_P_H

#include "dbusproperties.h"
#include "gattcharacteristic.h"

#include <memory>

namespace BluezQt
{

typedef org::freedesktop::DBus::Properties DBusProperties;

class GattServicePrivate;

class GattCharacterisiticPrivate
{
public:
    GattCharacterisiticPrivate(const QString &uuid, const QStringList &flags, const GattService *service);

    QString m_uuid;
    const GattService *m_service;
    bool m_notifying;
    QStringList m_flags;

    bool m_canNotify;
    QDBusObjectPath m_objectPath;
    QByteArray m_value;
    GattCharacteristic::ReadCallback m_readCallback = nullptr;
    std::unique_ptr<DBusProperties> m_dbusProperties;

    void emitPropertyChanged(QVariantMap updatedProperties);
};

} // namespace BluezQt

#endif

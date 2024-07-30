/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2022 Pontus Sjögren
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTDESCRIPTOR_P_H
#define GATTDESCRIPTOR_P_H

#include <QString>

#include <QDBusObjectPath>

namespace BluezQt
{

class GattCharacteristic;

class GattDescriptorPrivate
{
public:
    GattDescriptorPrivate(const QString &uuid, const QStringList &flags, const QByteArray &initialValue, const GattCharacteristic *characteristic);

    QString m_uuid;
    const GattCharacteristic *m_characteristic;
    QDBusObjectPath m_objectPath;
    QByteArray m_value;
    QStringList m_flags;
};

}

#endif

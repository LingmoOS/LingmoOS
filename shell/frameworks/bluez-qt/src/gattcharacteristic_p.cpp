/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattcharacteristic_p.h"
#include "gattservice.h"
#include "utils.h"

namespace BluezQt
{
GattCharacterisiticPrivate::GattCharacterisiticPrivate(const QString &uuid, const QStringList &flags, const GattService *service)
    : m_uuid(uuid)
    , m_service(service)
    , m_notifying(false)
    , m_flags(flags)
{
    static uint8_t charcNumber = 0;
    m_objectPath.setPath(m_service->objectPath().path() + QStringLiteral("/char") + QString::number(charcNumber++));
    m_dbusProperties = std::make_unique<DBusProperties>(Strings::orgBluez(), m_objectPath.path(), DBusConnection::orgBluez());

    m_canNotify = flags.contains(QLatin1String("notify")) || flags.contains(QLatin1String("encrypt-notify"))
        || flags.contains(QLatin1String("encrypt-authenticated-notify")) || flags.contains(QLatin1String("secure-notify"));
}

void GattCharacterisiticPrivate::emitPropertyChanged(QVariantMap updatedProperties)
{
    QDBusMessage signal = QDBusMessage::createSignal(m_objectPath.path(), Strings::orgFreedesktopDBusProperties(), QStringLiteral("PropertiesChanged"));
    signal << Strings::orgBluezGattCharacteristic1();
    signal << updatedProperties;
    signal << QStringList();
    DBusConnection::orgBluez().send(signal);
}

} // namespace BluezQt

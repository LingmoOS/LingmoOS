/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemtime.h"

#include <QDBusConnection>
#include <QDBusMessage>

ModemTime::ModemTime(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_enabledNotifications(false)
{
}

ModemTime::~ModemTime()
{
}

QVariantMap ModemTime::networkTimezone() const
{
    return m_timezone;
}

QString ModemTime::GetNetworkTime()
{
    return QStringLiteral("2010-08-06T18:34:14.3+02:00");
}

void ModemTime::setModemPath(const QString &path)
{
    m_modemPath = path;
}

void ModemTime::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void ModemTime::setNetworkTimezone(const QVariantMap &timezone)
{
    m_timezone = timezone;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("NetworkTimezone"), QVariant::fromValue<QVariantMap>(m_timezone));
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Time") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

QVariantMap ModemTime::toMap() const
{
    QVariantMap map;
    map.insert(QLatin1String(MM_MODEM_TIME_PROPERTY_NETWORKTIMEZONE), QVariant::fromValue<QVariantMap>(m_timezone));
    return map;
}

#include "moc_modemtime.cpp"

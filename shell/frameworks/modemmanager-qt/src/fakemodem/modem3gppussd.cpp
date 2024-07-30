/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem3gppussd.h"

#include <QDBusConnection>
#include <QDBusMessage>

Modem3gppUssd::Modem3gppUssd(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_enabledNotifications(false)
    , m_state(0)
{
}

Modem3gppUssd::~Modem3gppUssd()
{
}

QString Modem3gppUssd::networkNotification() const
{
    return m_networkNotification;
}

QString Modem3gppUssd::networkRequest() const
{
    return m_networkRequest;
}

uint Modem3gppUssd::state() const
{
    return m_state;
}

void Modem3gppUssd::Cancel()
{
}

QString Modem3gppUssd::Initiate(const QString &command)
{
    Q_UNUSED(command);
    return QString();
}

QString Modem3gppUssd::Respond(const QString &response)
{
    Q_UNUSED(response);
    return QString();
}

void Modem3gppUssd::setModemPath(const QString &path)
{
    m_modemPath = path;
}

void Modem3gppUssd::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void Modem3gppUssd::setNetworkNotification(const QString &networkNotification)
{
    m_networkNotification = networkNotification;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("NetworkNotification"), m_networkNotification);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp.Ussd") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem3gppUssd::setNetworkRequest(const QString &networkRequest)
{
    m_networkRequest = networkRequest;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("NetworkRequest"), m_networkRequest);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp.Ussd") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem3gppUssd::setState(uint state)
{
    m_state = state;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("State"), m_state);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp.Ussd") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

QVariantMap Modem3gppUssd::toMap() const
{
    QVariantMap map;
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_USSD_PROPERTY_STATE), m_state);
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_USSD_PROPERTY_NETWORKNOTIFICATION), m_networkNotification);
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_USSD_PROPERTY_NETWORKREQUEST), m_networkRequest);
    return map;
}

#include "moc_modem3gppussd.cpp"

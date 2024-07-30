/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem3gpp.h"

#include <QDBusConnection>
#include <QDBusMessage>

Modem3gpp::Modem3gpp(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_enabledNotifications(false)
    , m_enabledFacilityLocks(0)
    , m_registrationState(0)
    , m_subscriptionState(0)
{
}

Modem3gpp::~Modem3gpp()
{
}

uint Modem3gpp::enabledFacilityLocks() const
{
    return m_enabledFacilityLocks;
}

QString Modem3gpp::imei() const
{
    return m_imei;
}

QString Modem3gpp::operatorCode() const
{
    return m_operatorCode;
}

QString Modem3gpp::operatorName() const
{
    return m_operatorName;
}

uint Modem3gpp::registrationState() const
{
    return m_registrationState;
}

uint Modem3gpp::subscriptionState() const
{
    return m_subscriptionState;
}

void Modem3gpp::Register(const QString &operator_id)
{
    Q_UNUSED(operator_id);
}

ModemManager::QVariantMapList Modem3gpp::Scan()
{
    return ModemManager::QVariantMapList();
}

void Modem3gpp::setModemPath(const QString &path)
{
    m_modemPath = path;
}

void Modem3gpp::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void Modem3gpp::setEnabledFacilityLocks(uint enabledFacilityLocks)
{
    m_enabledFacilityLocks = enabledFacilityLocks;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("EnabledFacilityLocks"), m_enabledFacilityLocks);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem3gpp::setImei(const QString &imei)
{
    m_imei = imei;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Imei"), m_imei);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem3gpp::setOperatorCode(const QString &operatorCode)
{
    m_operatorCode = operatorCode;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("OperatorCode"), m_operatorCode);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem3gpp::setOperatorName(const QString &operatorName)
{
    m_operatorName = operatorName;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("OperatorName"), m_operatorName);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem3gpp::setRegistrationState(uint registrationState)
{
    m_registrationState = registrationState;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("RegistrationState"), m_registrationState);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void Modem3gpp::setSubscriptionState(uint subscriptionState)
{
    m_subscriptionState = subscriptionState;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SubscriptionState"), m_subscriptionState);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Modem3gpp") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

QVariantMap Modem3gpp::toMap() const
{
    QVariantMap map;
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_IMEI), m_imei);
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_REGISTRATIONSTATE), m_registrationState);
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_OPERATORCODE), m_operatorCode);
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_OPERATORNAME), m_operatorName);
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_ENABLEDFACILITYLOCKS), m_enabledFacilityLocks);
#if MM_CHECK_VERSION(1, 2, 0)
    map.insert(QLatin1String(MM_MODEM_MODEM3GPP_PROPERTY_SUBSCRIPTIONSTATE), m_subscriptionState);
#endif
    return map;
}

#include "moc_modem3gpp.cpp"

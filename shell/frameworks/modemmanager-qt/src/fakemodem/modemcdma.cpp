/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemcdma.h"

#include <QDBusConnection>
#include <QDBusMessage>

ModemCdma::ModemCdma(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_enabledNotifications(false)
    , m_activationState(0)
    , m_cdma1xRegistrationState(0)
    , m_evdoRegistrationState(0)
    , m_nid(0)
    , m_sid(0)
{
}

ModemCdma::~ModemCdma()
{
}

uint ModemCdma::activationState() const
{
    return m_activationState;
}

uint ModemCdma::cdma1xRegistrationState() const
{
    return m_cdma1xRegistrationState;
}

QString ModemCdma::esn() const
{
    return m_esn;
}

uint ModemCdma::evdoRegistrationState() const
{
    return m_evdoRegistrationState;
}

QString ModemCdma::meid() const
{
    return m_meid;
}

uint ModemCdma::nid() const
{
    return m_nid;
}

uint ModemCdma::sid() const
{
    return m_sid;
}

void ModemCdma::Activate(const QString &carrier_code)
{
    Q_UNUSED(carrier_code);
}

void ModemCdma::ActivateManual(const QVariantMap &properties)
{
    Q_UNUSED(properties);
}

void ModemCdma::setModemPath(const QString &path)
{
    m_modemPath = path;
}

void ModemCdma::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void ModemCdma::setActivationState(uint state)
{
    m_activationState = state;

    if (m_enabledNotifications) {
        Q_EMIT ActivationStateChanged(state, MM_CDMA_ACTIVATION_ERROR_NONE, QVariantMap());
    }
}

void ModemCdma::setCdma1xRegistrationState(uint state)
{
    m_cdma1xRegistrationState = state;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Cdma1xRegistrationState"), m_cdma1xRegistrationState);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.ModemCdma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemCdma::setEsn(const QString &esn)
{
    m_esn = esn;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Esn"), m_esn);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.ModemCdma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemCdma::setEvdoRegistrationState(uint state)
{
    m_evdoRegistrationState = state;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("EvdoRegistrationState"), m_evdoRegistrationState);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.ModemCdma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemCdma::setMeid(const QString &meid)
{
    m_meid = meid;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Meid"), m_meid);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.ModemCdma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemCdma::setNid(uint nid)
{
    m_nid = nid;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Nid"), m_nid);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.ModemCdma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemCdma::setSid(uint sid)
{
    m_sid = sid;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Sid"), m_sid);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.ModemCdma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

QVariantMap ModemCdma::toMap() const
{
    QVariantMap map;
    map.insert(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_ACTIVATIONSTATE), m_activationState);
    map.insert(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_MEID), m_meid);
    map.insert(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_ESN), m_esn);
    map.insert(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_SID), m_sid);
    map.insert(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_NID), m_nid);
    map.insert(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_CDMA1XREGISTRATIONSTATE), m_cdma1xRegistrationState);
    map.insert(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_EVDOREGISTRATIONSTATE), m_evdoRegistrationState);
    return map;
}

#include "moc_modemcdma.cpp"

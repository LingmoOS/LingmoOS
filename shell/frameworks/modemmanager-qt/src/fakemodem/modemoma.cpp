/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemoma.h"

#include <QDBusConnection>
#include <QDBusMessage>

ModemOma::ModemOma(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_enabledNotifications(false)
    , m_features(0)
    , m_sessionState(0)
    , m_sessionType(0)
{
}

ModemOma::~ModemOma()
{
}

uint ModemOma::features() const
{
    return m_features;
}

ModemManager::OmaSessionTypes ModemOma::pendingNetworkInitiatedSessions() const
{
    return m_sessions;
}

int ModemOma::sessionState() const
{
    return m_sessionState;
}

uint ModemOma::sessionType() const
{
    return m_sessionType;
}

void ModemOma::AcceptNetworkInitiatedSession(uint session_id, bool accept)
{
    Q_UNUSED(session_id);
    Q_UNUSED(accept);
}

void ModemOma::CancelSession()
{
}

void ModemOma::Setup(uint features)
{
    Q_UNUSED(features);
}

void ModemOma::StartClientInitiatedSession(uint session_type)
{
    Q_UNUSED(session_type);
}

void ModemOma::setModemPath(const QString &path)
{
    m_modemPath = path;
}

void ModemOma::setEnableNotifications(bool enable)
{
    m_enabledNotifications = enable;
}

void ModemOma::setFeatures(uint features)
{
    m_features = features;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("Features"), m_features);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Oma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemOma::setPendingNetworkInitiatedSessions(const ModemManager::OmaSessionTypes &sessions)
{
    m_sessions = sessions;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("PendingNetworkInitiatedSessions"), QVariant::fromValue<ModemManager::OmaSessionTypes>(m_sessions));
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Oma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

void ModemOma::setSessionState(int state)
{
    int previousState = m_sessionState;
    m_sessionState = state;

    if (m_enabledNotifications) {
        Q_EMIT SessionStateChanged(previousState, m_sessionState, MM_OMA_SESSION_STATE_UNKNOWN);
    }
}

void ModemOma::setSessionType(uint type)
{
    m_sessionType = type;

    if (m_enabledNotifications) {
        QVariantMap map;
        map.insert(QLatin1String("SessionType"), m_sessionType);
        QDBusMessage message = QDBusMessage::createSignal(m_modemPath, QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("PropertiesChanged"));
        message << QLatin1String("org.kde.fakemodem.Modem.Oma") << map << QStringList();
        QDBusConnection::sessionBus().send(message);
    }
}

QVariantMap ModemOma::toMap() const
{
    QVariantMap map;
    map.insert(QLatin1String(MM_MODEM_OMA_PROPERTY_FEATURES), m_features);
    map.insert(QLatin1String(MM_MODEM_OMA_PROPERTY_PENDINGNETWORKINITIATEDSESSIONS), QVariant::fromValue<ModemManager::OmaSessionTypes>(m_sessions));
    map.insert(QLatin1String(MM_MODEM_OMA_PROPERTY_SESSIONTYPE), m_sessionType);
    map.insert(QLatin1String(MM_MODEM_OMA_PROPERTY_SESSIONSTATE), m_sessionState);
    return map;
}

#include "moc_modemoma.cpp"

/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "settings.h"

#include <QDBusConnection>
#include <QDBusMetaType>

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_canModify(true)
    , m_hostname(QLatin1String("fake-hostname"))
    , m_connectionCounter(0)
{
    qDBusRegisterMetaType<NMVariantMapMap>();
}

Settings::~Settings()
{
    for (auto it = m_connections.cbegin(); it != m_connections.cend(); ++it) {
        const QDBusObjectPath &connection = it.key();
        QDBusConnection::sessionBus().unregisterObject(connection.path());
        Q_EMIT ConnectionRemoved(connection);
    }

    qDeleteAll(m_connections);
}

bool Settings::canModify() const
{
    return m_canModify;
}

QList<QDBusObjectPath> Settings::connections() const
{
    return m_connections.keys();
}

QString Settings::hostname() const
{
    return m_hostname;
}

QDBusObjectPath Settings::AddConnection(const NMVariantMapMap &connection)
{
    Connection *newConnection = new Connection(this, connection);
    QString newConnectionPath = QString("/org/kde/fakenetwork/Settings/") + QString::number(m_connectionCounter++);
    newConnection->setConnectionPath(newConnectionPath);
    m_connections.insert(QDBusObjectPath(newConnectionPath), newConnection);
    QDBusConnection::sessionBus().registerObject(newConnectionPath, newConnection, QDBusConnection::ExportScriptableContents);

    connect(newConnection, &Connection::connectionRemoved, this, &Settings::onConnectionRemoved);

    Q_EMIT NewConnection(QDBusObjectPath(newConnectionPath));
    // Send it for FakeNetwork separately to get AvailableConnections signal after NewConnection
    Q_EMIT connectionAdded(QDBusObjectPath(newConnectionPath));

    return QDBusObjectPath(newConnectionPath);
}

QDBusObjectPath Settings::AddConnectionUnsaved(const NMVariantMapMap &connection)
{
    Q_UNUSED(connection)
    // TODO
    return QDBusObjectPath();
}

QDBusObjectPath Settings::GetConnectionByUuid(const QString &uuid)
{
    Q_UNUSED(uuid)
    // TODO
    return QDBusObjectPath();
}

QList<QDBusObjectPath> Settings::ListConnections()
{
    return m_connections.keys();
}

void Settings::SaveHostname(const QString &hostname)
{
    m_hostname = hostname;
}

void Settings::onConnectionRemoved(const QDBusObjectPath &connectionPath)
{
    Connection *connection = m_connections.value(connectionPath);

    if (connection) {
        QDBusConnection::sessionBus().unregisterObject(connectionPath.path());
        Q_EMIT ConnectionRemoved(connectionPath);
        // Send it for FakeNetwork separately to get AvailableConnections signal after ConnectionRemoved
        Q_EMIT connectionRemoved(connectionPath);
        m_connections.remove(QDBusObjectPath(connectionPath));
        delete connection;
    }
}

#include "moc_settings.cpp"

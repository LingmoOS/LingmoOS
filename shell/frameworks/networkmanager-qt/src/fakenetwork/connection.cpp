/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connection.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>

Connection::Connection(QObject *parent, const NMVariantMapMap &settings)
    : QObject(parent)
    , m_unsaved(false)
    , m_settings(settings)
{
    qDBusRegisterMetaType<NMVariantMapMap>();
}

Connection::~Connection()
{
}

bool Connection::unsaved() const
{
    return m_unsaved;
}

void Connection::Delete()
{
    // From some reason signal Removed is not send over DBus when using only Q_EMIT
    QDBusMessage message = QDBusMessage::createSignal(m_connectionPath, QLatin1String("org.kde.fakenetwork.Settings.Connection"), QLatin1String("Removed"));
    QDBusConnection::sessionBus().send(message);
    // Q_EMIT Removed();

    // Send it for FakeNetwork as well
    Q_EMIT connectionRemoved(QDBusObjectPath(m_connectionPath));
}

NMVariantMapMap Connection::GetSecrets(const QString &setting_name)
{
    Q_UNUSED(setting_name)
    // TODO
    return NMVariantMapMap();
}

NMVariantMapMap Connection::GetSettings()
{
    // TODO: return settings without secrets
    return m_settings;
}

void Connection::Save()
{
    // TODO
}

void Connection::Update(const NMVariantMapMap &properties)
{
    m_settings = properties;

    Q_EMIT Updated();
}

void Connection::UpdateUnsaved(const NMVariantMapMap &properties)
{
    Q_UNUSED(properties)
    // TODO
}

QString Connection::connectionPath() const
{
    return m_connectionPath;
}

void Connection::setConnectionPath(const QString &path)
{
    m_connectionPath = path;
}

#include "moc_connection.cpp"

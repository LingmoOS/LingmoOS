/*
    SPDX-FileCopyrightText: 2008, 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connection_p.h"

#undef signals
#include <libnm/NetworkManager.h>
#define signals Q_SIGNALS

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>

#include "nmdebug.h"

NetworkManager::ConnectionPrivate::ConnectionPrivate(const QString &path, Connection *q)
#ifdef NMQT_STATIC
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
}

NetworkManager::Connection::Connection(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new ConnectionPrivate(path, this))
{
    Q_D(Connection);

    qDBusRegisterMetaType<NMVariantMapMap>();
    QDBusReply<NMVariantMapMap> reply = d->iface.GetSettings();
    if (reply.isValid()) {
        d->updateSettings(reply.value());
    } else {
        d->updateSettings();
    }
    d->path = path;
    // qCDebug(NMQT) << m_connection;

    connect(&d->iface, &OrgFreedesktopNetworkManagerSettingsConnectionInterface::Updated, d, &ConnectionPrivate::onConnectionUpdated);
    connect(&d->iface, &OrgFreedesktopNetworkManagerSettingsConnectionInterface::Removed, d, &ConnectionPrivate::onConnectionRemoved);

    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         d->path,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         d,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));
    d->unsaved = d->iface.unsaved();
}

NetworkManager::Connection::~Connection()
{
    delete d_ptr;
}

bool NetworkManager::Connection::isValid() const
{
    Q_D(const Connection);
    return d->iface.isValid();
}

QString NetworkManager::Connection::uuid() const
{
    Q_D(const Connection);
    return d->uuid;
}

QString NetworkManager::Connection::name() const
{
    Q_D(const Connection);
    return d->id;
}

bool NetworkManager::Connection::isUnsaved() const
{
    Q_D(const Connection);
    return d->unsaved;
}

NetworkManager::ConnectionSettings::Ptr NetworkManager::Connection::settings()
{
    Q_D(Connection);

    if (d->connection.isNull()) {
        d->connection = ConnectionSettings::Ptr(new ConnectionSettings(d->settings));
    }
    return d->connection;
}

QDBusPendingReply<NMVariantMapMap> NetworkManager::Connection::secrets(const QString &setting)
{
    Q_D(Connection);
    return d->iface.GetSecrets(setting);
}

QDBusPendingReply<> NetworkManager::Connection::update(const NMVariantMapMap &settings)
{
    Q_D(Connection);
    return d->iface.Update(settings);
}

QDBusPendingReply<> NetworkManager::Connection::updateUnsaved(const NMVariantMapMap &settings)
{
    Q_D(Connection);
    return d->iface.UpdateUnsaved(settings);
}

QDBusPendingReply<> NetworkManager::Connection::save()
{
    Q_D(Connection);
    return d->iface.Save();
}

QDBusPendingReply<> NetworkManager::Connection::clearSecrets()
{
    Q_D(Connection);
    return d->iface.ClearSecrets();
}

QDBusPendingReply<> NetworkManager::Connection::remove()
{
    Q_D(Connection);
    return d->iface.Delete();
}

QString NetworkManager::Connection::path() const
{
    Q_D(const Connection);
    return d->path;
}

void NetworkManager::ConnectionPrivate::onConnectionUpdated()
{
    Q_Q(Connection);
    QDBusReply<NMVariantMapMap> reply = iface.GetSettings();
    if (reply.isValid()) {
        updateSettings(reply.value());
    } else {
        updateSettings();
    }
    Q_EMIT q->updated();
}

void NetworkManager::ConnectionPrivate::onConnectionRemoved()
{
    Q_Q(Connection);
    QString tmpPath = path;
    updateSettings();
    Q_EMIT q->removed(tmpPath);
}

void NetworkManager::ConnectionPrivate::dbusPropertiesChanged(const QString &interfaceName,
                                                              const QVariantMap &properties,
                                                              const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == QLatin1String("org.freedesktop.NetworkManager.Settings.Connection")) {
        onPropertiesChanged(properties);
    }
}

void NetworkManager::ConnectionPrivate::onPropertiesChanged(const QVariantMap &properties)
{
    Q_Q(Connection);
    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString property = it.key();
        if (property == QLatin1String("Unsaved")) {
            unsaved = it->toBool();
            Q_EMIT q->unsavedChanged(unsaved);
        } else {
            qCWarning(NMQT) << Q_FUNC_INFO << "Unhandled property" << property;
        }
        ++it;
    }
}

void NetworkManager::ConnectionPrivate::updateSettings(const NMVariantMapMap &newSettings)
{
    settings = newSettings;
    if (settings.contains(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME))) {
        QVariantMap connectionSetting = settings.value(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
        if (connectionSetting.contains(QLatin1String(NM_SETTING_CONNECTION_UUID))) {
            uuid = connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_UUID)).toString();
        }
        if (connectionSetting.contains(QLatin1String(NM_SETTING_CONNECTION_ID))) {
            id = connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_ID)).toString();
        }
    } else if (newSettings.isEmpty()) {
        uuid.clear();
        id.clear();
    }
    connection.clear();
}

#include "moc_connection.cpp"
#include "moc_connection_p.cpp"

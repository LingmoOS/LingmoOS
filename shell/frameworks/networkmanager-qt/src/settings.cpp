/*
    SPDX-FileCopyrightText: 2011-2013 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "settings.h"
#include "macros.h"
#include "manager_p.h"
#include "settings_p.h"

#include <QDBusObjectPath>
#include <QTimer>

#include <nm-setting-connection.h>

#include "nmdebug.h"

// NM_GLOBAL_STATIC(NetworkManager::SettingsPrivate, globalSettings)
Q_GLOBAL_STATIC(NetworkManager::SettingsPrivate, globalSettings)

NetworkManager::SettingsPrivate::SettingsPrivate()
#ifdef NMQT_STATIC
    : iface(NetworkManagerPrivate::DBUS_SERVICE, NetworkManagerPrivate::DBUS_SETTINGS_PATH, QDBusConnection::sessionBus())
#else
    : iface(NetworkManagerPrivate::DBUS_SERVICE, NetworkManagerPrivate::DBUS_SETTINGS_PATH, QDBusConnection::systemBus())
#endif
    , m_canModify(true)
{
    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         NetworkManagerPrivate::DBUS_SETTINGS_PATH,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         this,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));
    connect(&iface, &OrgFreedesktopNetworkManagerSettingsInterface::NewConnection, this, &SettingsPrivate::onConnectionAdded);
    connect(&iface,
            &OrgFreedesktopNetworkManagerSettingsInterface::ConnectionRemoved,
            this,
            static_cast<void (SettingsPrivate::*)(const QDBusObjectPath &)>(&SettingsPrivate::onConnectionRemoved));
    init();
    // This class is a friend of NetworkManagerPrivate thus initted there too
    // because of the init chain we must follow,
    // But if this class is used first we need to make sure the
    // NetworkManagerPrivate also get created so we have its signals for
    // when the daemon dies, we just can not call it directly here or
    // we will have a constructor infinite loop
    QTimer::singleShot(0, this, SLOT(initNotifier()));
}

void NetworkManager::SettingsPrivate::init()
{
    const QList<QDBusObjectPath> connectionList = iface.connections();
    qCDebug(NMQT) << "Connections list";
    for (const QDBusObjectPath &connection : connectionList) {
        if (!connections.contains(connection.path())) {
            connections.insert(connection.path(), Connection::Ptr());
            Q_EMIT connectionAdded(connection.path());
            qCDebug(NMQT) << " " << connection.path();
        }
    }

    // Get all Setting's properties at once
    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(iface.staticInterfaceName(), NetworkManagerPrivate::DBUS_SETTINGS_PATH);
    if (!initialProperties.isEmpty()) {
        propertiesChanged(initialProperties);
    }
}

NetworkManager::Connection::List NetworkManager::SettingsPrivate::listConnections()
{
    NetworkManager::Connection::List list;
    QMap<QString, Connection::Ptr>::const_iterator i = connections.constBegin();
    while (i != connections.constEnd()) {
        NetworkManager::Connection::Ptr connection = findRegisteredConnection(i.key());
        if (connection) {
            list << connection;
        }
        ++i;
    }
    return list;
}

NetworkManager::Connection::Ptr NetworkManager::SettingsPrivate::findConnectionByUuid(const QString &uuid)
{
    QMap<QString, Connection::Ptr>::const_iterator i = connections.constBegin();
    while (i != connections.constEnd()) {
        NetworkManager::Connection::Ptr connection = findRegisteredConnection(i.key());
        if (connection && connection->uuid() == uuid) {
            return connection;
        }
        ++i;
    }

    return NetworkManager::Connection::Ptr();
}

QString NetworkManager::SettingsPrivate::hostname() const
{
    return m_hostname;
}

bool NetworkManager::SettingsPrivate::canModify() const
{
    return m_canModify;
}

QDBusPendingReply<QDBusObjectPath> NetworkManager::SettingsPrivate::addConnection(const NMVariantMapMap &connection)
{
    return iface.AddConnection(connection);
}

QDBusPendingReply<QDBusObjectPath> NetworkManager::SettingsPrivate::addConnectionUnsaved(const NMVariantMapMap &connection)
{
    return iface.AddConnectionUnsaved(connection);
}

QDBusPendingReply<bool, QStringList> NetworkManager::SettingsPrivate::loadConnections(const QStringList &filenames)
{
    return iface.LoadConnections(filenames);
}

QDBusPendingReply<bool> NetworkManager::SettingsPrivate::reloadConnections()
{
    return iface.ReloadConnections();
}

void NetworkManager::SettingsPrivate::initNotifier()
{
    notifier();
}

void NetworkManager::SettingsPrivate::saveHostname(const QString &hostname)
{
    iface.SaveHostname(hostname);
}

void NetworkManager::SettingsPrivate::dbusPropertiesChanged(const QString &interfaceName,
                                                            const QVariantMap &properties,
                                                            const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == QLatin1String("org.freedesktop.NetworkManager.Settings")) {
        propertiesChanged(properties);
    }
}

void NetworkManager::SettingsPrivate::propertiesChanged(const QVariantMap &properties)
{
    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString property = it.key();
        if (property == QLatin1String("CanModify")) {
            m_canModify = it->toBool();
            Q_EMIT canModifyChanged(m_canModify);
        } else if (property == QLatin1String("Hostname")) {
            m_hostname = it->toString();
            Q_EMIT hostnameChanged(m_hostname);
        } else if (property == QLatin1String("Connections")) {
            // will never get here in runtime NM < 0.9.10
            // TODO some action??
        } else {
            qCWarning(NMQT) << Q_FUNC_INFO << "Unhandled property" << property;
        }
        ++it;
    }
}

void NetworkManager::SettingsPrivate::onConnectionAdded(const QDBusObjectPath &path)
{
    const QString id = path.path();
    if (connections.contains(id)) {
        return;
    }
    connections.insert(id, Connection::Ptr());
    Q_EMIT connectionAdded(id);
}

NetworkManager::Connection::Ptr NetworkManager::SettingsPrivate::findRegisteredConnection(const QString &path)
{
    Connection::Ptr ret;
    if (!path.isEmpty()) {
        bool contains = connections.contains(path);
        if (contains && connections.value(path)) {
            ret = connections.value(path);
        } else {
            ret = Connection::Ptr(new Connection(path), &QObject::deleteLater);
            connections[path] = ret;
            connect(ret.data(), SIGNAL(removed(QString)), this, SLOT(onConnectionRemoved(QString)));
            if (!contains) {
                Q_EMIT connectionAdded(path);
            }
        }
    }
    return ret;
}

void NetworkManager::SettingsPrivate::onConnectionRemoved(const QDBusObjectPath &path)
{
    onConnectionRemoved(path.path());
}

void NetworkManager::SettingsPrivate::onConnectionRemoved(const QString &path)
{
    connections.remove(path);
    Q_EMIT connectionRemoved(path);
}

void NetworkManager::SettingsPrivate::daemonUnregistered()
{
    connections.clear();
}

NetworkManager::Connection::List NetworkManager::listConnections()
{
    return globalSettings->listConnections();
}

NetworkManager::Connection::Ptr NetworkManager::findConnectionByUuid(const QString &uuid)
{
    return globalSettings->findConnectionByUuid(uuid);
}

NetworkManager::Connection::Ptr NetworkManager::findConnection(const QString &path)
{
    return globalSettings->findRegisteredConnection(path);
}

QDBusPendingReply<QDBusObjectPath> NetworkManager::addConnection(const NMVariantMapMap &connection)
{
    return globalSettings->addConnection(connection);
}

QDBusPendingReply<QDBusObjectPath> NetworkManager::addConnectionUnsaved(const NMVariantMapMap &connection)
{
    return globalSettings->addConnectionUnsaved(connection);
}

QDBusPendingReply<bool, QStringList> NetworkManager::loadConnections(const QStringList &filenames)
{
    return globalSettings->loadConnections(filenames);
}

QDBusPendingReply<bool> NetworkManager::reloadConnections()
{
    return globalSettings->reloadConnections();
}

void NetworkManager::saveHostname(const QString &hostname)
{
    globalSettings->saveHostname(hostname);
}

bool NetworkManager::canModify()
{
    return globalSettings->canModify();
}

QString NetworkManager::hostname()
{
    return globalSettings->hostname();
}

NetworkManager::SettingsNotifier *NetworkManager::settingsNotifier()
{
    return globalSettings;
}

#include "moc_settings.cpp"
#include "moc_settings_p.cpp"

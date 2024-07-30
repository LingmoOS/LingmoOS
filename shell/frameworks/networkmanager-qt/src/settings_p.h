/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SETTINGS_P_H
#define NETWORKMANAGERQT_SETTINGS_P_H

#include <QDBusObjectPath>

#include "dbus/settingsinterface.h"
#include "settings.h"

class QDBusPendingCallWatcher;

namespace NetworkManager
{
class SettingsPrivate : public NetworkManager::SettingsNotifier
{
    Q_OBJECT
    friend class NetworkManagerPrivate;

public:
    SettingsPrivate();
    Connection::List listConnections();
    NetworkManager::Connection::Ptr findConnectionByUuid(const QString &uuid);
    QString hostname() const;
    bool canModify() const;
    QDBusPendingReply<QDBusObjectPath> addConnection(const NMVariantMapMap &);
    QDBusPendingReply<QDBusObjectPath> addConnectionUnsaved(const NMVariantMapMap &);
    QDBusPendingReply<bool, QStringList> loadConnections(const QStringList &filenames);
    void saveHostname(const QString &);
    QDBusPendingReply<bool> reloadConnections();
    Connection::Ptr findRegisteredConnection(const QString &);

    OrgFreedesktopNetworkManagerSettingsInterface iface;
    QMap<QString, Connection::Ptr> connections;
    bool m_canModify;
    QString m_hostname;
protected Q_SLOTS:
    void onConnectionAdded(const QDBusObjectPath &);
    void onConnectionRemoved(const QDBusObjectPath &);
    void onConnectionRemoved(const QString &);
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void propertiesChanged(const QVariantMap &properties);
    void initNotifier();

protected:
    void daemonUnregistered();
    void init();
};

}

#endif

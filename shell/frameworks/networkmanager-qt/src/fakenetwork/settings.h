/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_SETTINGS_H
#define NETWORKMANAGERQT_FAKE_NETWORK_SETTINGS_H

#include <QObject>

#include <QDBusObjectPath>

#include "../generictypes.h"

#include "connection.h"

class Settings : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork.Settings")
public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override;

    Q_PROPERTY(bool CanModify READ canModify)
    Q_PROPERTY(QList<QDBusObjectPath> Connections READ connections)
    Q_PROPERTY(QString Hostname READ hostname)

    bool canModify() const;
    QList<QDBusObjectPath> connections() const;
    QString hostname() const;

private Q_SLOTS:
    void onConnectionRemoved(const QDBusObjectPath &connectionPath);

public Q_SLOTS: // METHODS
    Q_SCRIPTABLE QDBusObjectPath AddConnection(const NMVariantMapMap &connection);
    Q_SCRIPTABLE QDBusObjectPath AddConnectionUnsaved(const NMVariantMapMap &connection);
    Q_SCRIPTABLE QDBusObjectPath GetConnectionByUuid(const QString &uuid);
    Q_SCRIPTABLE QList<QDBusObjectPath> ListConnections();
    Q_SCRIPTABLE void SaveHostname(const QString &hostname);

Q_SIGNALS:
    void connectionAdded(const QDBusObjectPath &connection);
    void connectionRemoved(const QDBusObjectPath &connection);
    Q_SCRIPTABLE void ConnectionRemoved(const QDBusObjectPath &connection);
    Q_SCRIPTABLE void NewConnection(const QDBusObjectPath &connection);
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);

private:
    bool m_canModify;
    QMap<QDBusObjectPath, Connection *> m_connections;
    QString m_hostname;

    /* Not part of DBus interface */
    int m_connectionCounter;
};

#endif

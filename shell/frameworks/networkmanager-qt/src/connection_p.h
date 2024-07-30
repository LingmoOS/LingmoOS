/*
    SPDX-FileCopyrightText: 2008, 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SETTINGS_CONNECTION_P_H
#define NETWORKMANAGERQT_SETTINGS_CONNECTION_P_H

#include "connection.h"
#include "connectioninterface.h"
#include "device.h"
#include "generictypes.h"
#include "manager_p.h"

class QDBusPendingCallWatcher;

namespace NetworkManager
{
class ConnectionPrivate : public QObject
{
    Q_OBJECT
public:
    ConnectionPrivate(const QString &path, Connection *q);

    void updateSettings(const NMVariantMapMap &newSettings = NMVariantMapMap());
    bool unsaved;
    QString uuid;
    QString id;
    NMVariantMapMap settings;
    ConnectionSettings::Ptr connection;
    QString path;
    OrgFreedesktopNetworkManagerSettingsConnectionInterface iface;

    Q_DECLARE_PUBLIC(Connection)
    Connection *q_ptr;
private Q_SLOTS:
    void onConnectionUpdated();
    void onConnectionRemoved();
    void dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties);
    void onPropertiesChanged(const QVariantMap &properties);
};

}

#endif

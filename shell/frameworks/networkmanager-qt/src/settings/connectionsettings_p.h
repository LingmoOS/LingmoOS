/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_CONNECTION_SETTINGS_P_H
#define NETWORKMANAGERQT_CONNECTION_SETTINGS_P_H

#include "setting.h"

#include <QDateTime>
#include <QHash>
#include <QString>

namespace NetworkManager
{
class ConnectionSettingsPrivate
{
public:
    Q_DECLARE_PUBLIC(ConnectionSettings)

    explicit ConnectionSettingsPrivate(ConnectionSettings *q);

    void addSetting(const NetworkManager::Setting::Ptr &setting);
    void clearSettings();
    void initSettings(NMBluetoothCapabilities bt_cap);
    void initSettings(const NetworkManager::ConnectionSettings::Ptr &connectionSettings);

    QString name;
    QString id;
    QString uuid;
    QString interfaceName;
    NetworkManager::ConnectionSettings::ConnectionType type;
    QHash<QString, QString> permissions;
    bool autoconnect;
    QDateTime timestamp;
    bool readOnly;
    QString zone;
    QString master;
    QString slaveType;
    QStringList secondaries;
    quint32 gatewayPingTimeout;
    int autoconnectPriority;
    int autoconnectRetries;
    NetworkManager::ConnectionSettings::AutoconnectSlaves autoconnectSlaves;
    NetworkManager::ConnectionSettings::Lldp lldp;
    NetworkManager::ConnectionSettings::Metered metered;
    NetworkManager::ConnectionSettings::Mdns mdns;
    QString stableId;
    Setting::List settings;

    ConnectionSettings *q_ptr;
};

}

#endif //  NETWORKMANAGERQT_CONNECTION_SETTINGS_P_H

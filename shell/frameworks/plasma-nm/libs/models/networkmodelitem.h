/*
    SPDX-FileCopyrightText: 2013-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_MODEL_NETWORK_MODEL_ITEM_H
#define PLASMA_NM_MODEL_NETWORK_MODEL_ITEM_H

#include "plasmanm_internal_export.h"

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnConnection>

class PLASMANM_INTERNAL_EXPORT NetworkModelItem : public QObject
{
    Q_OBJECT
public:
    enum ItemType { UnavailableConnection, AvailableConnection, AvailableAccessPoint };

    explicit NetworkModelItem(QObject *parent = nullptr);
    explicit NetworkModelItem(const NetworkModelItem *item, QObject *parent = nullptr);
    ~NetworkModelItem() override;

    QString activeConnectionPath() const;
    void setActiveConnectionPath(const QString &path);

    QString connectionPath() const;
    void setConnectionPath(const QString &path);

    NetworkManager::ActiveConnection::State connectionState() const;
    void setConnectionState(NetworkManager::ActiveConnection::State state);

    QStringList details() const;

    QString deviceName() const;
    void setDeviceName(const QString &name);

    QString devicePath() const;
    void setDevicePath(const QString &path);

    QString deviceState() const;
    void setDeviceState(const NetworkManager::Device::State state);

    bool duplicate() const;

    void setIcon(const QString &icon);
    QString icon() const
    {
        return m_icon;
    }

    ItemType itemType() const;

    NetworkManager::WirelessSetting::NetworkMode mode() const;
    void setMode(const NetworkManager::WirelessSetting::NetworkMode mode);

    QString name() const;
    void setName(const QString &name);

    QString originalName() const;

    QString sectionType() const;

    NetworkManager::WirelessSecurityType securityType() const;
    void setSecurityType(NetworkManager::WirelessSecurityType type);

    int signal() const;
    void setSignal(int signal);

    bool slave() const;
    void setSlave(bool slave);

    QString specificPath() const;
    void setSpecificPath(const QString &path);

    QString ssid() const;
    void setSsid(const QString &ssid);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &date);

    NetworkManager::ConnectionSettings::ConnectionType type() const;
    void setType(NetworkManager::ConnectionSettings::ConnectionType type);

    QString accessibleDescription() const;

    QString uni() const;

    QString uuid() const;
    void setUuid(const QString &uuid);

    QString vpnState() const;
    void setVpnState(NetworkManager::VpnConnection::State state);

    QString vpnType() const;
    void setVpnType(const QString &type);

    qulonglong rxBytes() const;
    void setRxBytes(qulonglong bytes);

    qulonglong txBytes() const;
    void setTxBytes(qulonglong bytes);

    bool delayModelUpdates() const;
    void setDelayModelUpdates(bool delay);

    bool operator==(const NetworkModelItem *item) const;

    QList<int> changedRoles() const
    {
        return m_changedRoles;
    }
    void clearChangedRoles()
    {
        m_changedRoles.clear();
    }

public Q_SLOTS:
    void invalidateDetails();

private:
    QString computeIcon() const;
    void refreshIcon();
    void updateDetails() const;

    QString m_activeConnectionPath;
    QString m_connectionPath;
    NetworkManager::ActiveConnection::State m_connectionState;
    QString m_devicePath;
    QString m_deviceName;
    NetworkManager::Device::State m_deviceState;
    mutable QStringList m_details;
    mutable bool m_detailsValid;
    bool m_delayModelUpdates;
    bool m_duplicate;
    NetworkManager::WirelessSetting::NetworkMode m_mode;
    QString m_name;
    NetworkManager::WirelessSecurityType m_securityType;
    int m_signal;
    bool m_slave;
    QString m_specificPath;
    QString m_ssid;
    QDateTime m_timestamp;
    NetworkManager::ConnectionSettings::ConnectionType m_type;
    QString m_accessibleDescription;
    QString m_uuid;
    QString m_vpnType;
    NetworkManager::VpnConnection::State m_vpnState;
    qulonglong m_rxBytes;
    qulonglong m_txBytes;
    QString m_icon;
    QList<int> m_changedRoles;
};

#endif // PLASMA_NM_MODEL_NETWORK_MODEL_ITEM_H

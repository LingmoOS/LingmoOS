/*
    SPDX-FileCopyrightText: 2013-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_NETWORK_MODEL_H
#define PLASMA_NM_NETWORK_MODEL_H

#include "plasmanm_internal_export.h"

#include <QAbstractListModel>
#include <QQueue>

#include <qqmlregistration.h>

#include "networkitemslist.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/WirelessDevice>

#include <ModemManagerQt/Modem>

class PLASMANM_INTERNAL_EXPORT NetworkModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    // Delay model updates to prevent jumping password entry in the applet.
    // This is an aggregate property, which is true whenever any of model items is true.
    // BUG: 435430, 389052
    Q_PROPERTY(bool delayModelUpdates READ delayModelUpdates NOTIFY delayModelUpdatesChanged)

    explicit NetworkModel(QObject *parent = nullptr);
    ~NetworkModel() override;

    enum ItemRole {
        ConnectionDetailsRole = Qt::UserRole + 1,
        ConnectionIconRole,
        ConnectionPathRole,
        ConnectionStateRole,
        DeviceName,
        DevicePathRole,
        DeviceStateRole,
        DuplicateRole,
        ItemUniqueNameRole,
        ItemTypeRole,
        LastUsedRole,
        LastUsedDateOnlyRole,
        NameRole,
        SecurityTypeRole,
        SecurityTypeStringRole,
        SectionRole,
        SignalRole,
        SlaveRole,
        SsidRole,
        SpecificPathRole,
        TimeStampRole,
        TypeRole,
        UniRole,
        UuidRole,
        VpnState,
        VpnType,
        RxBytesRole,
        TxBytesRole,
        // writable roles
        DelayModelUpdatesRole,
    };
    Q_ENUMS(ItemRole)

    enum ModelChangeType { ItemAdded, ItemRemoved, ItemPropertyChanged };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    bool delayModelUpdates() const;

Q_SIGNALS:
    void delayModelUpdatesChanged();

public Q_SLOTS:
    void onItemUpdated();
    void setDeviceStatisticsRefreshRateMs(const QString &devicePath, uint refreshRate);

private Q_SLOTS:
    void accessPointSignalStrengthChanged(int signal);
    void activeConnectionAdded(const QString &activeConnection);
    void activeConnectionRemoved(const QString &activeConnection);
    void activeConnectionStateChanged(NetworkManager::ActiveConnection::State state);
    void activeVpnConnectionStateChanged(NetworkManager::VpnConnection::State state, NetworkManager::VpnConnection::StateChangeReason reason);
    void availableConnectionAppeared(const QString &connection);
    void availableConnectionDisappeared(const QString &connection);
    void connectionAdded(const QString &connection);
    void connectionRemoved(const QString &connection);
    void connectionUpdated();
    void deviceAdded(const QString &device);
    void deviceRemoved(const QString &device);
    void deviceStateChanged(NetworkManager::Device::State state, NetworkManager::Device::State oldState, NetworkManager::Device::StateChangeReason reason);
    void gsmNetworkAccessTechnologiesChanged(QFlags<MMModemAccessTechnology> accessTechnologies);
    void gsmNetworkCurrentModesChanged();
    void gsmNetworkSignalQualityChanged(const ModemManager::SignalQualityPair &signalQuality);
    void ipConfigChanged();
    void ipInterfaceChanged();
    void statusChanged(NetworkManager::Status status);
    void wirelessNetworkAppeared(const QString &ssid);
    void wirelessNetworkDisappeared(const QString &ssid);
    void wirelessNetworkSignalChanged(int signal);
    void wirelessNetworkReferenceApChanged(const QString &accessPoint);

    void initialize();

private:
    bool m_delayModelUpdates = false;
    NetworkItemsList m_list;
    QQueue<QPair<ModelChangeType, NetworkModelItem *>> m_updateQueue;

    void addActiveConnection(const NetworkManager::ActiveConnection::Ptr &activeConnection);
    void addAvailableConnection(const QString &connection, const NetworkManager::Device::Ptr &device);
    void addConnection(const NetworkManager::Connection::Ptr &connection);
    void addDevice(const NetworkManager::Device::Ptr &device);
    void addWirelessNetwork(const NetworkManager::WirelessNetwork::Ptr &network, const NetworkManager::WirelessDevice::Ptr &device);
    void checkAndCreateDuplicate(const QString &connection, const QString &deviceUni);
    void initializeSignals();
    void initializeSignals(const NetworkManager::ActiveConnection::Ptr &activeConnection);
    void initializeSignals(const NetworkManager::Connection::Ptr &connection);
    void initializeSignals(const NetworkManager::Device::Ptr &device);
    void initializeSignals(const NetworkManager::WirelessNetwork::Ptr &network);
    void
    updateFromWirelessNetwork(NetworkModelItem *item, const NetworkManager::WirelessNetwork::Ptr &network, const NetworkManager::WirelessDevice::Ptr &device);
    void updateDelayModelUpdates();
    void flushUpdateQueue();

    void insertItem(NetworkModelItem *item);
    void removeItem(NetworkModelItem *item);
    void updateItem(NetworkModelItem *item);

    NetworkManager::WirelessSecurityType alternativeWirelessSecurity(const NetworkManager::WirelessSecurityType type);
};

#endif // PLASMA_NM_NETWORK_MODEL_H

/*
    SPDX-FileCopyrightText: 2013-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "networkmodel.h"
#include "configuration.h"
#include "networkmodelitem.h"
#include "plasma_nm_libs.h"
#include "uiutils.h"
#include <QList>
#include <algorithm>

#include <ModemManagerQt/Manager>

#include <NetworkManagerQt/GenericDevice>
#include <NetworkManagerQt/Settings>

NetworkModel::NetworkModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QLoggingCategory::setFilterRules(QStringLiteral("plasma-nm.debug = false"));

    initialize();
}

NetworkModel::~NetworkModel() = default;

QVariant NetworkModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();

    if (row >= 0 && row < m_list.count()) {
        NetworkModelItem *item = m_list.itemAt(row);

        switch (role) {
        case ConnectionDetailsRole:
            return item->details();
        case ConnectionIconRole:
            return item->icon();
        case ConnectionPathRole:
            return item->connectionPath();
        case ConnectionStateRole:
            return item->connectionState();
        case DeviceName:
            return item->deviceName();
        case DevicePathRole:
            return item->devicePath();
        case DeviceStateRole:
            return item->deviceState();
        case DuplicateRole:
            return item->duplicate();
        case ItemUniqueNameRole:
            if (m_list.returnItems(NetworkItemsList::Name, item->name()).count() > 1) {
                return item->originalName();
            } else {
                return item->name();
            }
        case ItemTypeRole:
            return item->itemType();
        case LastUsedRole:
            return UiUtils::formatLastUsedDateRelative(item->timestamp());
        case LastUsedDateOnlyRole:
            return UiUtils::formatDateRelative(item->timestamp());
        case NameRole:
            return item->name();
        case SectionRole:
            return item->sectionType();
        case SignalRole:
            return item->signal();
        case SlaveRole:
            return item->slave();
        case SsidRole:
            return item->ssid();
        case SpecificPathRole:
            return item->specificPath();
        case SecurityTypeRole:
            return item->securityType();
        case SecurityTypeStringRole:
            return UiUtils::labelFromWirelessSecurity(item->securityType());
        case TimeStampRole:
            return item->timestamp();
        case TypeRole:
            return item->type();
        case UniRole:
            return item->uni();
        case UuidRole:
            return item->uuid();
        case VpnState:
            return item->vpnState();
        case VpnType:
            return item->vpnType();
        case RxBytesRole:
            return item->rxBytes();
        case TxBytesRole:
            return item->txBytes();
        case DelayModelUpdatesRole:
            return item->delayModelUpdates();
        case Qt::AccessibleDescriptionRole:
            return item->accessibleDescription();
        default:
            break;
        }
    }

    return {};
}

bool NetworkModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const bool delay = value.toBool();

    if (row >= 0 && row < m_list.count() && role == DelayModelUpdatesRole) {
        NetworkModelItem *item = m_list.itemAt(row);
        if (item->delayModelUpdates() != delay) {
            item->setDelayModelUpdates(delay);
            dataChanged(index, index, QList<int>{DelayModelUpdatesRole});
            updateDelayModelUpdates();
            return true;
        }
    }
    return false;
}

int NetworkModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_list.count();
}

QHash<int, QByteArray> NetworkModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[ConnectionDetailsRole] = "ConnectionDetails";
    roles[ConnectionIconRole] = "ConnectionIcon";
    roles[ConnectionPathRole] = "ConnectionPath";
    roles[ConnectionStateRole] = "ConnectionState";
    roles[DeviceName] = "DeviceName";
    roles[DevicePathRole] = "DevicePath";
    roles[DeviceStateRole] = "DeviceState";
    roles[DuplicateRole] = "Duplicate";
    roles[ItemUniqueNameRole] = "ItemUniqueName";
    roles[ItemTypeRole] = "ItemType";
    roles[LastUsedRole] = "LastUsed";
    roles[LastUsedDateOnlyRole] = "LastUsedDateOnly";
    roles[NameRole] = "Name";
    roles[SectionRole] = "Section";
    roles[SignalRole] = "Signal";
    roles[SlaveRole] = "Slave";
    roles[SsidRole] = "Ssid";
    roles[SpecificPathRole] = "SpecificPath";
    roles[SecurityTypeRole] = "SecurityType";
    roles[SecurityTypeStringRole] = "SecurityTypeString";
    roles[TimeStampRole] = "TimeStamp";
    roles[TypeRole] = "Type";
    roles[Qt::AccessibleDescriptionRole] = "AccessibleDescription";
    roles[UniRole] = "Uni";
    roles[UuidRole] = "Uuid";
    roles[VpnState] = "VpnState";
    roles[VpnType] = "VpnType";
    roles[RxBytesRole] = "RxBytes";
    roles[TxBytesRole] = "TxBytes";
    roles[DelayModelUpdatesRole] = "DelayModelUpdates";

    return roles;
}

bool NetworkModel::delayModelUpdates() const
{
    return m_delayModelUpdates;
}

void NetworkModel::updateDelayModelUpdates()
{
    const QList<NetworkModelItem *> items = m_list.items();
    const bool delay = std::any_of(items.begin(), items.end(), [](NetworkModelItem *item) -> bool {
        return item->delayModelUpdates();
    });
    if (m_delayModelUpdates != delay) {
        m_delayModelUpdates = delay;
        Q_EMIT delayModelUpdatesChanged();

        if (!m_delayModelUpdates) {
            flushUpdateQueue();
        }
    }
}

void NetworkModel::flushUpdateQueue()
{
    while (!m_updateQueue.isEmpty()) {
        QPair<NetworkModel::ModelChangeType, NetworkModelItem *> update = m_updateQueue.dequeue();
        if (update.first == ItemAdded) {
            insertItem(update.second);
        } else if (update.first == ItemRemoved) {
            removeItem(update.second);
        } else if (update.first == ItemPropertyChanged) {
            updateItem(update.second);
        }
    }
}

void NetworkModel::initialize()
{
    // Initialize existing connections
    for (const NetworkManager::Connection::Ptr &connection : NetworkManager::listConnections()) {
        addConnection(connection);
    }

    // Initialize existing devices
    for (const NetworkManager::Device::Ptr &dev : NetworkManager::networkInterfaces()) {
        if (!dev->managed()) {
            continue;
        }
        if (dev->type() == NetworkManager::Device::Type::Loopback) {
            continue;
        }
        addDevice(dev);
    }

    // Initialize existing active connections
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections()) {
        addActiveConnection(active);
    }

    initializeSignals();
}

void NetworkModel::initializeSignals()
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionAdded, this, &NetworkModel::activeConnectionAdded, Qt::UniqueConnection);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionRemoved, this, &NetworkModel::activeConnectionRemoved, Qt::UniqueConnection);
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionAdded, this, &NetworkModel::connectionAdded, Qt::UniqueConnection);
    connect(NetworkManager::settingsNotifier(),
            &NetworkManager::SettingsNotifier::connectionRemoved,
            this,
            &NetworkModel::connectionRemoved,
            Qt::UniqueConnection);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &NetworkModel::deviceAdded, Qt::UniqueConnection);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &NetworkModel::deviceRemoved, Qt::UniqueConnection);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &NetworkModel::statusChanged, Qt::UniqueConnection);
}

void NetworkModel::initializeSignals(const NetworkManager::ActiveConnection::Ptr &activeConnection)
{
    if (activeConnection->vpn()) {
        NetworkManager::VpnConnection::Ptr vpnConnection = activeConnection.objectCast<NetworkManager::VpnConnection>();
        if (vpnConnection) {
            connect(vpnConnection.data(),
                    &NetworkManager::VpnConnection::stateChanged,
                    this,
                    &NetworkModel::activeVpnConnectionStateChanged,
                    Qt::UniqueConnection);
        }
    } else {
        connect(activeConnection.data(),
                &NetworkManager::ActiveConnection::stateChanged,
                this,
                &NetworkModel::activeConnectionStateChanged,
                Qt::UniqueConnection);
    }
}

void NetworkModel::initializeSignals(const NetworkManager::Connection::Ptr &connection)
{
    connect(connection.data(), &NetworkManager::Connection::updated, this, &NetworkModel::connectionUpdated, Qt::UniqueConnection);
}

void NetworkModel::initializeSignals(const NetworkManager::Device::Ptr &device)
{
    connect(device.data(), &NetworkManager::Device::availableConnectionAppeared, this, &NetworkModel::availableConnectionAppeared, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::Device::availableConnectionDisappeared, this, &NetworkModel::availableConnectionDisappeared, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::Device::ipV4ConfigChanged, this, &NetworkModel::ipConfigChanged, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::Device::ipV6ConfigChanged, this, &NetworkModel::ipConfigChanged, Qt::UniqueConnection);
    connect(device.data(), &NetworkManager::Device::ipInterfaceChanged, this, &NetworkModel::ipInterfaceChanged);
    connect(device.data(), &NetworkManager::Device::stateChanged, this, &NetworkModel::deviceStateChanged, Qt::UniqueConnection);

    auto deviceStatistics = device->deviceStatistics();
    connect(deviceStatistics.data(), &NetworkManager::DeviceStatistics::rxBytesChanged, this, [this, device](qulonglong rxBytes) {
        for (auto item : m_list.returnItems(NetworkItemsList::Device, device->uni())) {
            item->setRxBytes(rxBytes);
            updateItem(item);
        }
    });
    connect(deviceStatistics.data(), &NetworkManager::DeviceStatistics::txBytesChanged, this, [this, device](qulonglong txBytes) {
        for (auto item : m_list.returnItems(NetworkItemsList::Device, device->uni())) {
            item->setTxBytes(txBytes);
            updateItem(item);
        }
    });

    if (device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wifiDev = device.objectCast<NetworkManager::WirelessDevice>();
        connect(wifiDev.data(), &NetworkManager::WirelessDevice::networkAppeared, this, &NetworkModel::wirelessNetworkAppeared, Qt::UniqueConnection);
        connect(wifiDev.data(), &NetworkManager::WirelessDevice::networkDisappeared, this, &NetworkModel::wirelessNetworkDisappeared, Qt::UniqueConnection);

    } else if (device->type() == NetworkManager::Device::Modem) {
        ModemManager::ModemDevice::Ptr modem = ModemManager::findModemDevice(device->udi());
        if (modem) {
            if (modem->hasInterface(ModemManager::ModemDevice::ModemInterface)) {
                ModemManager::Modem::Ptr modemNetwork = modem->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
                if (modemNetwork) {
                    connect(modemNetwork.data(),
                            &ModemManager::Modem::signalQualityChanged,
                            this,
                            &NetworkModel::gsmNetworkSignalQualityChanged,
                            Qt::UniqueConnection);
                    connect(modemNetwork.data(),
                            &ModemManager::Modem::accessTechnologiesChanged,
                            this,
                            &NetworkModel::gsmNetworkAccessTechnologiesChanged,
                            Qt::UniqueConnection);
                    connect(modemNetwork.data(),
                            &ModemManager::Modem::currentModesChanged,
                            this,
                            &NetworkModel::gsmNetworkCurrentModesChanged,
                            Qt::UniqueConnection);
                }
            }
        }
    }
}

void NetworkModel::initializeSignals(const NetworkManager::WirelessNetwork::Ptr &network)
{
    connect(network.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, this, &NetworkModel::wirelessNetworkSignalChanged, Qt::UniqueConnection);
    connect(network.data(),
            &NetworkManager::WirelessNetwork::referenceAccessPointChanged,
            this,
            &NetworkModel::wirelessNetworkReferenceApChanged,
            Qt::UniqueConnection);
}

void NetworkModel::addActiveConnection(const NetworkManager::ActiveConnection::Ptr &activeConnection)
{
    initializeSignals(activeConnection);

    NetworkManager::Device::Ptr device;
    NetworkManager::Connection::Ptr connection = activeConnection->connection();

    // Not necessary to have device for VPN connections
    if (activeConnection && !activeConnection->vpn() && !activeConnection->devices().isEmpty()) {
        device = NetworkManager::findNetworkInterface(activeConnection->devices().first());
    }

    // Check whether we have a base connection
    if (!m_list.contains(NetworkItemsList::Uuid, connection->uuid())) {
        // Active connection appeared before a base connection, so we have to add its base connection first
        addConnection(connection);
    }

    for (const auto items = m_list.returnItems(NetworkItemsList::NetworkItemsList::Uuid, connection->uuid()); NetworkModelItem * item : items) {
        if (((device && device->uni() == item->devicePath()) || item->devicePath().isEmpty()) || item->type() == NetworkManager::ConnectionSettings::Vpn) {
            item->setActiveConnectionPath(activeConnection->path());
            item->setConnectionState(activeConnection->state());
            if (activeConnection->vpn()) {
                NetworkManager::VpnConnection::Ptr vpnConnection = activeConnection.objectCast<NetworkManager::VpnConnection>();
                NetworkManager::VpnConnection::State state = vpnConnection->state();
                if (state == NetworkManager::VpnConnection::Prepare //
                    || state == NetworkManager::VpnConnection::NeedAuth //
                    || state == NetworkManager::VpnConnection::Connecting //
                    || state == NetworkManager::VpnConnection::GettingIpConfig) {
                    item->setConnectionState(NetworkManager::ActiveConnection::Activating);
                } else if (state == NetworkManager::VpnConnection::Activated) {
                    item->setConnectionState(NetworkManager::ActiveConnection::Activated);
                } else {
                    item->setConnectionState(NetworkManager::ActiveConnection::Deactivated);
                }
                item->setVpnState(state);
            }
            qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": active connection state changed to " << item->connectionState();

            if (device && device->uni() == item->devicePath()) {
                auto deviceStatistics = device->deviceStatistics();
                if (deviceStatistics->refreshRateMs() != 0) {
                    item->setRxBytes(deviceStatistics->rxBytes());
                    item->setTxBytes(deviceStatistics->txBytes());
                }
            }
        }
        updateItem(item);
    }
}

void NetworkModel::addAvailableConnection(const QString &connection, const NetworkManager::Device::Ptr &device)
{
    if (!device) {
        return;
    }

    checkAndCreateDuplicate(connection, device->uni());

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Connection, connection)) {
        // The item is already associated with another device
        if (!device || !item->devicePath().isEmpty()) {
            continue;
        }

        if (device->ipInterfaceName().isEmpty()) {
            item->setDeviceName(device->interfaceName());
        } else {
            item->setDeviceName(device->ipInterfaceName());
        }

        item->setDevicePath(device->uni());
        item->setDeviceState(device->state());
        qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": device changed to " << item->devicePath();
        if (device->type() == NetworkManager::Device::Modem) {
            ModemManager::ModemDevice::Ptr modemDevice = ModemManager::findModemDevice(device->udi());
            if (modemDevice) {
                ModemManager::Modem::Ptr modemInterface = modemDevice->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
                if (modemInterface) {
                    item->setSignal(modemInterface->signalQuality().signal);
                    qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": signal changed to " << item->signal();
                }
            }
        }

        if (item->type() == NetworkManager::ConnectionSettings::Wireless && item->mode() == NetworkManager::WirelessSetting::Infrastructure) {
            // Find an accesspoint which could be removed, because it will be merged with a connection
            for (NetworkModelItem *secondItem : m_list.returnItems(NetworkItemsList::Ssid, item->ssid())) {
                if (secondItem->itemType() == NetworkModelItem::AvailableAccessPoint && secondItem->devicePath() == item->devicePath()) {
                    removeItem(secondItem);
                    qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Access point " << secondItem->name() << ": merged to " << item->name() << " connection";
                    break;
                }
            }

            NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
            if (wifiDevice) {
                NetworkManager::WirelessNetwork::Ptr wifiNetwork = wifiDevice->findNetwork(item->ssid());
                if (wifiNetwork) {
                    updateFromWirelessNetwork(item, wifiNetwork, wifiDevice);
                }
            }
        }

        updateItem(item);
        break;
    }
}

void NetworkModel::addConnection(const NetworkManager::Connection::Ptr &connection)
{
    // Can't add a connection without name or uuid
    if (connection->name().isEmpty() || connection->uuid().isEmpty()) {
        return;
    }

    initializeSignals(connection);

    NetworkManager::ConnectionSettings::Ptr settings = connection->settings();
    NetworkManager::VpnSetting::Ptr vpnSetting;
    NetworkManager::WirelessSetting::Ptr wirelessSetting;

    if (settings->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
        vpnSetting = settings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    } else if (settings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
        wirelessSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    }

    // Check whether the connection is already in the model to avoid duplicates, but this shouldn't happen
    if (m_list.contains(NetworkItemsList::Connection, connection->path())) {
        return;
    }

    auto item = new NetworkModelItem();
    item->setConnectionPath(connection->path());
    item->setName(settings->id());
    item->setTimestamp(settings->timestamp());
    item->setType(settings->connectionType());
    item->setUuid(settings->uuid());
    item->setSlave(settings->isSlave());

    if (item->type() == NetworkManager::ConnectionSettings::Vpn) {
        item->setVpnType(vpnSetting->serviceType().section('.', -1));
    } else if (item->type() == NetworkManager::ConnectionSettings::Wireless) {
        item->setMode(wirelessSetting->mode());
        item->setSecurityType(NetworkManager::securityTypeFromConnectionSetting(settings));
        item->setSsid(QString::fromUtf8(wirelessSetting->ssid()));
    }

    item->invalidateDetails();

    insertItem(item);
    qCDebug(PLASMA_NM_LIBS_LOG) << "New connection" << item->name() << "added";
}

void NetworkModel::addDevice(const NetworkManager::Device::Ptr &device)
{
    initializeSignals(device);

    if (device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wifiDev = device.objectCast<NetworkManager::WirelessDevice>();
        for (const NetworkManager::WirelessNetwork::Ptr &wifiNetwork : wifiDev->networks()) {
            addWirelessNetwork(wifiNetwork, wifiDev);
        }
    }

    for (const NetworkManager::Connection::Ptr &connection : device->availableConnections()) {
        addAvailableConnection(connection->path(), device);
    }
}

void NetworkModel::addWirelessNetwork(const NetworkManager::WirelessNetwork::Ptr &network, const NetworkManager::WirelessDevice::Ptr &device)
{
    initializeSignals(network);

    // Avoid duplicating entries in the model
    if (!Configuration::self().hotspotConnectionPath().isEmpty()) {
        NetworkManager::ActiveConnection::Ptr activeConnection = NetworkManager::findActiveConnection(Configuration::self().hotspotConnectionPath());

        // If we are trying to add an AP which is the one created by our hotspot, then we can skip this and don't add it twice
        if (activeConnection && activeConnection->specificObject() == network->referenceAccessPoint()->uni()) {
            return;
        }
    }

    // BUG: 386342
    // When creating a new hidden wireless network and attempting to connect to it, NM then later reports that AccessPoint appeared, but
    // it doesn't know its SSID from some reason, this also makes Wireless device to advertise a new available connection, which we later
    // attempt to merge with an AP, based on its SSID, but it doesn't find any, because we have AP with empty SSID. After this we get another
    // AccessPoint appeared signal, this time we know SSID, but we don't attempt any merging, because it's usually the other way around, thus
    // we need to attempt to merge it here with a connection we guess it's related to this new AP
    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Type, NetworkManager::ConnectionSettings::Wireless)) {
        if (item->itemType() != NetworkModelItem::AvailableConnection)
            continue;

        NetworkManager::ConnectionSettings::Ptr connectionSettings = NetworkManager::findConnection(item->connectionPath())->settings();
        if (connectionSettings && connectionSettings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
            NetworkManager::WirelessSetting::Ptr wirelessSetting =
                connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (QString::fromUtf8(wirelessSetting->ssid()) == network->ssid()) {
                const QString bssid = NetworkManager::macAddressAsString(wirelessSetting->bssid());
                const QString restrictedHw = NetworkManager::macAddressAsString(wirelessSetting->macAddress());
                if ((bssid.isEmpty() || bssid == network->referenceAccessPoint()->hardwareAddress())
                    && (restrictedHw.isEmpty() || restrictedHw == device->hardwareAddress())) {
                    updateFromWirelessNetwork(item, network, device);
                    return;
                }
            }
        }
    }

    NetworkManager::WirelessSetting::NetworkMode mode = NetworkManager::WirelessSetting::Infrastructure;
    NetworkManager::WirelessSecurityType securityType = NetworkManager::UnknownSecurity;

    NetworkManager::AccessPoint::Ptr ap = network->referenceAccessPoint();
    if (ap && (ap->capabilities().testFlag(NetworkManager::AccessPoint::Privacy) || ap->wpaFlags() || ap->rsnFlags())) {
        securityType = NetworkManager::findBestWirelessSecurity(device->wirelessCapabilities(),
                                                                true,
                                                                (device->mode() == NetworkManager::WirelessDevice::Adhoc),
                                                                ap->capabilities(),
                                                                ap->wpaFlags(),
                                                                ap->rsnFlags());
        if (network->referenceAccessPoint()->mode() == NetworkManager::AccessPoint::Infra) {
            mode = NetworkManager::WirelessSetting::Infrastructure;
        } else if (network->referenceAccessPoint()->mode() == NetworkManager::AccessPoint::Adhoc) {
            mode = NetworkManager::WirelessSetting::Adhoc;
        } else if (network->referenceAccessPoint()->mode() == NetworkManager::AccessPoint::ApMode) {
            mode = NetworkManager::WirelessSetting::Ap;
        }
    }

    auto item = new NetworkModelItem();
    if (device->ipInterfaceName().isEmpty()) {
        item->setDeviceName(device->interfaceName());
    } else {
        item->setDeviceName(device->ipInterfaceName());
    }
    item->setDevicePath(device->uni());
    item->setMode(mode);
    item->setName(network->ssid());
    item->setSignal(network->signalStrength());
    item->setSpecificPath(network->referenceAccessPoint()->uni());
    item->setSsid(network->ssid());
    item->setType(NetworkManager::ConnectionSettings::Wireless);
    item->setSecurityType(securityType);
    item->invalidateDetails();

    insertItem(item);
    qCDebug(PLASMA_NM_LIBS_LOG) << "New wireless network" << item->name() << "added";
}

void NetworkModel::checkAndCreateDuplicate(const QString &connection, const QString &deviceUni)
{
    bool createDuplicate = false;
    NetworkModelItem *originalItem = nullptr;

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Connection, connection)) {
        if (!item->duplicate()) {
            originalItem = item;
        }

        if (!item->duplicate() && item->itemType() == NetworkModelItem::AvailableConnection
            && (item->devicePath() != deviceUni && !item->devicePath().isEmpty())) {
            createDuplicate = true;
        }
    }

    if (createDuplicate) {
        auto duplicatedItem = new NetworkModelItem(originalItem);
        duplicatedItem->invalidateDetails();

        insertItem(duplicatedItem);
    }
}

void NetworkModel::onItemUpdated()
{
    auto item = static_cast<NetworkModelItem *>(sender());
    if (item) {
        updateItem(item);
    }
}

void NetworkModel::setDeviceStatisticsRefreshRateMs(const QString &devicePath, uint refreshRate)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(devicePath);

    if (device) {
        device->deviceStatistics()->setRefreshRateMs(refreshRate);
    }
}

void NetworkModel::insertItem(NetworkModelItem *item)
{
    if (m_delayModelUpdates) {
        m_updateQueue.enqueue(QPair<NetworkModel::ModelChangeType, NetworkModelItem *>(NetworkModel::ItemAdded, item));
        return;
    }

    const int index = m_list.count();
    beginInsertRows(QModelIndex(), index, index);
    m_list.insertItem(item);
    endInsertRows();
    updateDelayModelUpdates();
}

void NetworkModel::removeItem(NetworkModelItem *item)
{
    if (m_delayModelUpdates) {
        m_updateQueue.enqueue(QPair<NetworkModel::ModelChangeType, NetworkModelItem *>(NetworkModel::ItemRemoved, item));
        return;
    }

    const int row = m_list.indexOf(item);
    if (row != -1) {
        beginRemoveRows(QModelIndex(), row, row);
        m_list.removeItem(item);
        item->deleteLater();
        endRemoveRows();
        updateDelayModelUpdates();
    }
}

void NetworkModel::updateItem(NetworkModelItem *item)
{
    if (m_delayModelUpdates) {
        m_updateQueue.enqueue(QPair<NetworkModel::ModelChangeType, NetworkModelItem *>(NetworkModel::ItemPropertyChanged, item));
        return;
    }

    const int row = m_list.indexOf(item);
    if (row != -1) {
        item->invalidateDetails();
        QModelIndex index = createIndex(row, 0);
        Q_EMIT dataChanged(index, index, item->changedRoles());
        item->clearChangedRoles();
        updateDelayModelUpdates();
    }
}

void NetworkModel::accessPointSignalStrengthChanged(int signal)
{
    auto apPtr = qobject_cast<NetworkManager::AccessPoint *>(sender());
    if (!apPtr) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Ssid, apPtr->ssid())) {
        if (item->specificPath() == apPtr->uni()) {
            item->setSignal(signal);
            updateItem(item);
            qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "AccessPoint " << item->name() << ": signal changed to " << item->signal();
        }
    }
}

void NetworkModel::activeConnectionAdded(const QString &activeConnection)
{
    NetworkManager::ActiveConnection::Ptr activeCon = NetworkManager::findActiveConnection(activeConnection);

    if (activeCon) {
        addActiveConnection(activeCon);
    }
}

void NetworkModel::activeConnectionRemoved(const QString &activeConnection)
{
    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::ActiveConnection, activeConnection)) {
        item->setActiveConnectionPath(QString());
        item->setConnectionState(NetworkManager::ActiveConnection::Deactivated);
        item->setVpnState(NetworkManager::VpnConnection::Disconnected);
        updateItem(item);
        qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": active connection removed";
    }
}

void NetworkModel::activeConnectionStateChanged(NetworkManager::ActiveConnection::State state)
{
    auto activePtr = qobject_cast<NetworkManager::ActiveConnection *>(sender());

    if (!activePtr) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::ActiveConnection, activePtr->path())) {
        item->setConnectionState(state);
        updateItem(item);
        qCDebug(PLASMA_NM_LIBS_LOG) << "Item " << item->name() << ": active connection changed to " << item->connectionState();
    }
}

void NetworkModel::activeVpnConnectionStateChanged(NetworkManager::VpnConnection::State state, NetworkManager::VpnConnection::StateChangeReason reason)
{
    Q_UNUSED(reason)
    auto activePtr = qobject_cast<NetworkManager::ActiveConnection *>(sender());

    if (!activePtr) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::ActiveConnection, activePtr->path())) {
        if (state == NetworkManager::VpnConnection::Prepare //
            || state == NetworkManager::VpnConnection::NeedAuth //
            || state == NetworkManager::VpnConnection::Connecting //
            || state == NetworkManager::VpnConnection::GettingIpConfig) {
            item->setConnectionState(NetworkManager::ActiveConnection::Activating);
        } else if (state == NetworkManager::VpnConnection::Activated) {
            item->setConnectionState(NetworkManager::ActiveConnection::Activated);
        } else {
            item->setConnectionState(NetworkManager::ActiveConnection::Deactivated);
        }
        item->setVpnState(state);
        updateItem(item);
        qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": active connection changed to " << item->connectionState();
    }
}

void NetworkModel::availableConnectionAppeared(const QString &connection)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(qobject_cast<NetworkManager::Device *>(sender())->uni());
    if (!device) {
        return;
    }

    addAvailableConnection(connection, device);
}

void NetworkModel::availableConnectionDisappeared(const QString &connection)
{
    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Connection, connection)) {
        bool available = false;
        const QString devicePath = item->devicePath();
        const QString specificPath = item->specificPath();

        // We have to check whether the connection is still available, because it might be
        // presented in the model for more devices and we don't want to remove it for all of them.

        // Check whether the device is still available
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(devicePath);
        if (device) {
            // Check whether the connection is still listed as available
            for (const NetworkManager::Connection::Ptr &connection : device->availableConnections()) {
                if (connection->path() == item->connectionPath()) {
                    available = true;
                    break;
                }
            }
        }

        if (!available) {
            item->setDeviceName(QString());
            item->setDevicePath(QString());
            item->setDeviceState(NetworkManager::Device::UnknownState);
            item->setSignal(0);
            item->setSpecificPath(QString());
            qCDebug(PLASMA_NM_LIBS_LOG) << "Item" << item->name() << "removed as available connection";
            // Check whether the connection is still available as an access point, this happens
            // when we change its properties, like ssid, bssid, security etc.
            if (item->type() == NetworkManager::ConnectionSettings::Wireless && !specificPath.isEmpty()) {
                if (device && device->type() == NetworkManager::Device::Wifi) {
                    NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
                    if (wifiDevice) {
                        NetworkManager::AccessPoint::Ptr ap = wifiDevice->findAccessPoint(specificPath);
                        if (ap) {
                            NetworkManager::WirelessNetwork::Ptr network = wifiDevice->findNetwork(ap->ssid());
                            if (network) {
                                addWirelessNetwork(network, wifiDevice);
                            }
                        }
                    }
                }
            }

            if (item->duplicate()) {
                removeItem(item);
                qCDebug(PLASMA_NM_LIBS_LOG) << "Duplicate item" << item->name() << "removed completely";
            } else {
                updateItem(item);
            }
        }
        available = false;
    }
}

void NetworkModel::connectionAdded(const QString &connection)
{
    NetworkManager::Connection::Ptr newConnection = NetworkManager::findConnection(connection);
    if (newConnection) {
        addConnection(newConnection);
    }
}

void NetworkModel::connectionRemoved(const QString &connection)
{
    bool remove = false;
    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Connection, connection)) {
        // When the item type is wireless, we can remove only the connection and leave it as an available access point
        if (item->type() == NetworkManager::ConnectionSettings::Wireless && !item->devicePath().isEmpty()) {
            for (NetworkModelItem *secondItem : m_list.items()) {
                // Remove it entirely when there is another connection with the same configuration and for the same device
                // or it's a shared connection
                if ((item->mode() != NetworkManager::WirelessSetting::Infrastructure)
                    || (item->connectionPath() != secondItem->connectionPath() //
                        && item->devicePath() == secondItem->devicePath() //
                        && item->mode() == secondItem->mode() //
                        && item->securityType() == secondItem->securityType() //
                        && item->ssid() == secondItem->ssid())) {
                    remove = true;
                    break;
                }
            }

            if (!remove) {
                item->setConnectionPath(QString());
                item->setName(item->ssid());
                item->setSlave(false);
                item->setTimestamp(QDateTime());
                item->setUuid(QString());
                updateItem(item);
                qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": connection removed";
            }
        } else {
            remove = true;
        }

        if (remove) {
            removeItem(item);
            qCDebug(PLASMA_NM_LIBS_LOG) << "Item" << item->name() << "removed completely";
        }
        remove = false;
    }
}

void NetworkModel::connectionUpdated()
{
    auto connectionPtr = qobject_cast<NetworkManager::Connection *>(sender());
    if (!connectionPtr) {
        return;
    }

    NetworkManager::ConnectionSettings::Ptr settings = connectionPtr->settings();
    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Connection, connectionPtr->path())) {
        item->setConnectionPath(connectionPtr->path());
        item->setName(settings->id());
        item->setTimestamp(settings->timestamp());
        item->setType(settings->connectionType());
        item->setUuid(settings->uuid());

        if (item->type() == NetworkManager::ConnectionSettings::Wireless) {
            NetworkManager::WirelessSetting::Ptr wirelessSetting;
            wirelessSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            item->setMode(wirelessSetting->mode());
            item->setSecurityType(NetworkManager::securityTypeFromConnectionSetting(settings));
            item->setSsid(QString::fromUtf8(wirelessSetting->ssid()));
            // TODO check whether BSSID has changed and update the wireless info
        }

        updateItem(item);
        qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": connection updated";
    }
}

void NetworkModel::deviceAdded(const QString &device)
{
    NetworkManager::Device::Ptr dev = NetworkManager::findNetworkInterface(device);
    if (dev) {
        addDevice(dev);
    }
}

void NetworkModel::deviceRemoved(const QString &device)
{
    // Make all items unavailable
    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Device, device)) {
        availableConnectionDisappeared(item->connectionPath());
    }
}

void NetworkModel::deviceStateChanged(NetworkManager::Device::State state,
                                      NetworkManager::Device::State oldState,
                                      NetworkManager::Device::StateChangeReason reason)
{
    Q_UNUSED(oldState);
    Q_UNUSED(reason);

    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(qobject_cast<NetworkManager::Device *>(sender())->uni());

    if (!device) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Device, device->uni())) {
        item->setDeviceState(state);
        updateItem(item);
        // qCDebug(PLASMA_NM_LIBS_LOG) << "Item " << item->name() << ": device state changed to " << item->deviceState();
    }
}

void NetworkModel::gsmNetworkAccessTechnologiesChanged(QFlags<MMModemAccessTechnology> accessTechnologies)
{
    Q_UNUSED(accessTechnologies);
    auto gsmNetwork = qobject_cast<ModemManager::Modem *>(sender());
    if (!gsmNetwork)
        return;

    for (const NetworkManager::Device::Ptr &dev : NetworkManager::networkInterfaces()) {
        if (dev->type() != NetworkManager::Device::Modem) {
            continue;
        }

        ModemManager::ModemDevice::Ptr modem = ModemManager::findModemDevice(dev->udi());
        if (!modem) {
            continue;
        }

        if (!modem->hasInterface(ModemManager::ModemDevice::ModemInterface)) {
            continue;
        }

        ModemManager::Modem::Ptr modemNetwork = modem->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
        if (!modemNetwork || modemNetwork->device() != gsmNetwork->device()) {
            continue;
        }

        // TODO store access technology internally?
        for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Device, dev->uni())) {
            updateItem(item);
        }
    }
}

void NetworkModel::gsmNetworkCurrentModesChanged()
{
    auto gsmNetwork = qobject_cast<ModemManager::Modem *>(sender());
    if (!gsmNetwork) {
        return;
    }

    for (const NetworkManager::Device::Ptr &dev : NetworkManager::networkInterfaces()) {
        if (dev->type() != NetworkManager::Device::Modem)
            continue;

        ModemManager::ModemDevice::Ptr modem = ModemManager::findModemDevice(dev->udi());
        if (!modem) {
            continue;
        }

        if (!modem->hasInterface(ModemManager::ModemDevice::ModemInterface)) {
            continue;
        }

        ModemManager::Modem::Ptr modemNetwork = modem->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
        if (!modemNetwork || modemNetwork->device() != gsmNetwork->device()) {
            continue;
        }

        for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Device, dev->uni())) {
            updateItem(item);
        }
    }
}

void NetworkModel::gsmNetworkSignalQualityChanged(const ModemManager::SignalQualityPair &signalQuality)
{
    auto gsmNetwork = qobject_cast<ModemManager::Modem *>(sender());
    if (!gsmNetwork) {
        return;
    }

    for (const NetworkManager::Device::Ptr &dev : NetworkManager::networkInterfaces()) {
        if (dev->type() != NetworkManager::Device::Modem) {
            continue;
        }

        ModemManager::ModemDevice::Ptr modem = ModemManager::findModemDevice(dev->udi());
        if (!modem) {
            continue;
        }

        if (!modem->hasInterface(ModemManager::ModemDevice::ModemInterface)) {
            continue;
        }

        ModemManager::Modem::Ptr modemNetwork = modem->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
        if (!modemNetwork || modemNetwork->device() != gsmNetwork->device()) {
            continue;
        }

        for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Device, dev->uni())) {
            item->setSignal(signalQuality.signal);
            updateItem(item);
        }
    }
}

void NetworkModel::ipConfigChanged()
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(qobject_cast<NetworkManager::Device *>(sender())->uni());

    if (!device) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Device, device->uni())) {
        updateItem(item);
        // qCDebug(PLASMA_NM_LIBS_LOG) << "Item " << item->name() << ": device ipconfig changed";
    }
}

void NetworkModel::ipInterfaceChanged()
{
    auto device = qobject_cast<NetworkManager::Device *>(sender());
    if (!device) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Device, device->uni())) {
        if (device->ipInterfaceName().isEmpty()) {
            item->setDeviceName(device->interfaceName());
        } else {
            item->setDeviceName(device->ipInterfaceName());
        }
    }
}

void NetworkModel::statusChanged(NetworkManager::Status status)
{
    Q_UNUSED(status);

    qCDebug(PLASMA_NM_LIBS_LOG) << "NetworkManager state changed to" << status;
    // This has probably effect only for VPN connections
    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Type, NetworkManager::ConnectionSettings::Vpn)) {
        updateItem(item);
    }
}

void NetworkModel::wirelessNetworkAppeared(const QString &ssid)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(qobject_cast<NetworkManager::Device *>(sender())->uni());
    if (device && device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
        NetworkManager::WirelessNetwork::Ptr network = wirelessDevice->findNetwork(ssid);
        addWirelessNetwork(network, wirelessDevice);
    }
}

void NetworkModel::wirelessNetworkDisappeared(const QString &ssid)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(qobject_cast<NetworkManager::Device *>(sender())->uni());
    if (!device) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Ssid, ssid, device->uni())) {
        // Remove the entire item, because it's only AP or it's a duplicated available connection
        if (item->itemType() == NetworkModelItem::AvailableAccessPoint || item->duplicate()) {
            removeItem(item);
            qCDebug(PLASMA_NM_LIBS_LOG) << "Wireless network" << item->name() << "removed completely";
            // Remove only AP and device from the item and leave it as an unavailable connection
        } else {
            if (item->mode() == NetworkManager::WirelessSetting::Infrastructure) {
                item->setDeviceName(QString());
                item->setDevicePath(QString());
                item->setSpecificPath(QString());
            }
            item->setSignal(0);
            updateItem(item);
            qCDebug(PLASMA_NM_LIBS_LOG).nospace() << "Item " << item->name() << ": wireless network removed";
        }
    }
}

void NetworkModel::wirelessNetworkReferenceApChanged(const QString &accessPoint)
{
    auto networkPtr = qobject_cast<NetworkManager::WirelessNetwork *>(sender());

    if (!networkPtr) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Ssid, networkPtr->ssid(), networkPtr->device())) {
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(item->connectionPath());
        if (!connection) {
            continue;
        }

        NetworkManager::WirelessSetting::Ptr wirelessSetting =
            connection->settings()->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
        if (!wirelessSetting) {
            continue;
        }

        if (wirelessSetting->bssid().isEmpty()) {
            item->setSpecificPath(accessPoint);
            updateItem(item);
        }
    }
}

void NetworkModel::wirelessNetworkSignalChanged(int signal)
{
    auto networkPtr = qobject_cast<NetworkManager::WirelessNetwork *>(sender());
    if (!networkPtr) {
        return;
    }

    for (NetworkModelItem *item : m_list.returnItems(NetworkItemsList::Ssid, networkPtr->ssid(), networkPtr->device())) {
        if (item->specificPath() == networkPtr->referenceAccessPoint()->uni()) {
            item->setSignal(signal);
            updateItem(item);
            // qCDebug(PLASMA_NM_LIBS_LOG) << "Wireless network " << item->name() << ": signal changed to " << item->signal();
        }
    }
}

NetworkManager::WirelessSecurityType NetworkModel::alternativeWirelessSecurity(const NetworkManager::WirelessSecurityType type)
{
    if (type == NetworkManager::WpaPsk) {
        return NetworkManager::Wpa2Psk;
    } else if (type == NetworkManager::WpaEap) {
        return NetworkManager::Wpa2Eap;
    } else if (type == NetworkManager::Wpa2Psk) {
        return NetworkManager::WpaPsk;
    } else if (type == NetworkManager::Wpa2Eap) {
        return NetworkManager::WpaEap;
    }
    return type;
}

void NetworkModel::updateFromWirelessNetwork(NetworkModelItem *item,
                                             const NetworkManager::WirelessNetwork::Ptr &network,
                                             const NetworkManager::WirelessDevice::Ptr &device)
{
    NetworkManager::WirelessSecurityType securityType = NetworkManager::UnknownSecurity;
    NetworkManager::AccessPoint::Ptr ap = network->referenceAccessPoint();
    if (ap && ap->capabilities().testFlag(NetworkManager::AccessPoint::Privacy)) {
        securityType = NetworkManager::findBestWirelessSecurity(device->wirelessCapabilities(),
                                                                true,
                                                                (device->mode() == NetworkManager::WirelessDevice::Adhoc),
                                                                ap->capabilities(),
                                                                ap->wpaFlags(),
                                                                ap->rsnFlags());
    } else if (ap) {
        securityType = NetworkManager::NoneSecurity;
    }

    // Check whether the connection is associated with some concrete AP
    NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(item->connectionPath());
    if (connection) {
        NetworkManager::WirelessSetting::Ptr wirelessSetting =
            connection->settings()->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting) {
            if (!wirelessSetting->bssid().isEmpty()) {
                for (const NetworkManager::AccessPoint::Ptr &ap : network->accessPoints()) {
                    if (ap->hardwareAddress() == NetworkManager::macAddressAsString(wirelessSetting->bssid())) {
                        item->setSignal(ap->signalStrength());
                        item->setSpecificPath(ap->uni());
                        // We need to watch this AP for signal changes
                        connect(ap.data(),
                                &NetworkManager::AccessPoint::signalStrengthChanged,
                                this,
                                &NetworkModel::accessPointSignalStrengthChanged,
                                Qt::UniqueConnection);
                    }
                }
            } else {
                item->setSignal(network->signalStrength());
                item->setSpecificPath(network->referenceAccessPoint()->uni());
            }
        }
    }
    item->setSecurityType(securityType);
    updateItem(item);
}

#include "moc_networkmodel.cpp"

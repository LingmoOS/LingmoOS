/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2021 Alessio Bonfiglio <alessio.bonfiglio@mail.polimi.it>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "NetworkManagerBackend.h"

#include <numeric>

#include <QString>
#include <QTimer>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/ModemDevice>

// Update rate in ms
static const int UpdateRate = 500;

NetworkManagerDevice::NetworkManagerDevice(const QString &id, QSharedPointer<NetworkManager::Device> device)
    : NetworkDevice(id, id)
    , m_device(device)
{
    connect(m_device.data(), &NetworkManager::Device::activeConnectionChanged, this, &NetworkManagerDevice::update);
    connect(m_device.data(), &NetworkManager::Device::ipV4ConfigChanged, this, &NetworkManagerDevice::update);
    connect(m_device.data(), &NetworkManager::Device::ipV6ConfigChanged, this, &NetworkManagerDevice::update);

    connect(this, &NetworkManagerDevice::nameChanged, this, [this]() {
        m_networkSensor->setPrefix(name());
        m_signalSensor->setPrefix(name());
        m_ipv4Sensor->setPrefix(name());
        m_ipv4GatewaySensor->setPrefix(name());
        m_ipv4SubnetMaskSensor->setPrefix(name());
        m_ipv4WithPrefixLengthSensor->setPrefix(name());
        m_ipv4DNSSensor->setPrefix(name());
        m_ipv6Sensor->setPrefix(name());
        m_ipv6GatewaySensor->setPrefix(name());
        m_ipv6SubnetMaskSensor->setPrefix(name());
        m_ipv6WithPrefixLengthSensor->setPrefix(name());
        m_ipv6DNSSensor->setPrefix(name());
        m_downloadSensor->setPrefix(name());
        m_uploadSensor->setPrefix(name());
        m_downloadBitsSensor->setPrefix(name());
        m_uploadBitsSensor->setPrefix(name());
        m_totalDownloadSensor->setPrefix(name());
        m_totalUploadSensor->setPrefix(name());
    });

    m_statistics = m_device->deviceStatistics();

    // We always want to have the refresh rate to be 1000ms but it's a global property. So we store
    // the oustide rate. override any changes and restore it when we are destroyed.
    m_initialStatisticsRate = m_statistics->refreshRateMs();
    connect(m_statistics.get(), &NetworkManager::DeviceStatistics::refreshRateMsChanged, this, [this] (uint rate) {
        // Unfortunately we always get a change signal even when disconnecting before the setter and
        // connecting afterwards, so we have to do this and assume the first signal after a call is
        // caused by it. Iniitally true because of the call below
        static bool updatingRefreshRate = true;
        if (!updatingRefreshRate) {
            m_initialStatisticsRate = rate;
            m_statistics->setRefreshRateMs(UpdateRate);
        }
        updatingRefreshRate = !updatingRefreshRate;
    });

    // We want to display speed in bytes per second, so use a fixed one-second
    // update interval here so we are independant of the actual update rate of
    // the daemon.
    m_statistics->setRefreshRateMs(UpdateRate);

    // Unfortunately, the statistics interface does not emit change signals if
    // no change happened. This makes the change signals rather useless for our
    // case because we also need to know when no change happened, so that we
    // can update rate sensors to show 0. So instead use a timer and query the
    // statistics every UpdateRate ms, updating the sensors as needed.
    m_statisticsTimer = std::make_unique<QTimer>();
    m_statisticsTimer->setInterval(UpdateRate);
    connect(m_statisticsTimer.get(), &QTimer::timeout, this, [this]() {
        auto newDownload = m_statistics->rxBytes();
        auto previousDownload = m_totalDownloadSensor->value().toULongLong();
        if (previousDownload > 0) {
            m_downloadSensor->setValue((newDownload - previousDownload) * (1000 / UpdateRate));
            m_downloadBitsSensor->setValue((newDownload - previousDownload) * (1000 / UpdateRate) * 8);
        }
        m_totalDownloadSensor->setValue(newDownload);

        auto newUpload = m_statistics->txBytes();
        auto previousUpload = m_totalUploadSensor->value().toULongLong();
        if (previousUpload > 0) {
            m_uploadSensor->setValue((newUpload - previousUpload) * (1000 / UpdateRate));
            m_uploadBitsSensor->setValue((newUpload - previousUpload) * (1000 / UpdateRate) * 8);
        }
        m_totalUploadSensor->setValue(newUpload);
    });

    std::vector<KSysGuard::SensorProperty*> statisticSensors{m_downloadSensor, m_downloadBitsSensor, m_totalDownloadSensor, m_uploadSensor, m_uploadBitsSensor, m_totalUploadSensor};
    for (auto property : statisticSensors) {
        connect(property, &KSysGuard::SensorProperty::subscribedChanged, this, [this, statisticSensors](bool subscribed) {
            if (subscribed && !m_statisticsTimer->isActive()) {
                m_statisticsTimer->start();
            } else if (std::none_of(statisticSensors.begin(), statisticSensors.end(), [](auto property) { return property->isSubscribed(); })) {
                m_statisticsTimer->stop();
                m_totalDownloadSensor->setValue(0);
                m_totalUploadSensor->setValue(0);
            }
        });
    }

    if (m_device->type() == NetworkManager::Device::Wifi) {
        m_wifiDevice = m_device->as<NetworkManager::WirelessDevice>();
        connect(m_wifiDevice, &NetworkManager::WirelessDevice::activeConnectionChanged, this, &NetworkManagerDevice::updateWifi);
        connect(m_wifiDevice, &NetworkManager::WirelessDevice::networkAppeared, this, &NetworkManagerDevice::updateWifi);
        connect(m_wifiDevice, &NetworkManager::WirelessDevice::networkDisappeared, this, &NetworkManagerDevice::updateWifi);
        updateWifi();
    }

    update();
}

NetworkManagerDevice::~NetworkManagerDevice()
{
    disconnect(m_statistics.get(), nullptr, this, nullptr);
    m_statistics->setRefreshRateMs(m_initialStatisticsRate);
}

void NetworkManagerDevice::update()
{
    if (!m_device->activeConnection()) {
        if (m_connected) {
            m_connected = false;
            if (m_statisticsTimer->isActive()) {
                m_restoreTimer = true;
                m_statisticsTimer->stop();
            } else {
                m_restoreTimer = false;
            }
            Q_EMIT disconnected(this);
        }
        return;
    }

    if (m_device->activeConnection() && !m_connected) {
        m_connected = true;
        if (m_restoreTimer) {
            m_statisticsTimer->start();
        }

        Q_EMIT connected(this);
    }

    setName(m_device->activeConnection()->connection()->name());
    m_networkSensor->setValue(name());

    auto dnsAccumulationFunction = [](QString &&a, const QHostAddress& b) {return a.append(", ").append(b.toString()); };
    if (m_device->ipV4Config().isValid()) {
        auto ipv4 = m_device->ipV4Config().addresses().at(0).ip().toString();
        m_ipv4Sensor->setValue(ipv4);
        m_ipv4GatewaySensor->setValue(m_device->ipV4Config().gateway());
        m_ipv4SubnetMaskSensor->setValue(m_device->ipV4Config().addresses().at(0).netmask().toString());
        m_ipv4WithPrefixLengthSensor->setValue(static_cast<QString>(ipv4 + '/' + QString::number(m_device->ipV4Config().addresses().at(0).prefixLength())));
        auto dnsList = m_device->ipV4Config().nameservers();
        auto dnsListString = QString();
        if(!dnsList.isEmpty()) {
            dnsListString = std::accumulate(std::next(dnsList.begin()), dnsList.end(), dnsList.at(0).toString(),
                                            dnsAccumulationFunction);
        }
        m_ipv4DNSSensor->setValue(dnsListString);
    } else {
        m_ipv4Sensor->setValue(QString{});
        m_ipv4GatewaySensor->setValue(QString{});
        m_ipv4SubnetMaskSensor->setValue(QString{});
        m_ipv4WithPrefixLengthSensor->setValue(QString{});
        m_ipv4DNSSensor->setValue(QString{});
    }

    if (m_device->ipV6Config().isValid()) {
        auto ipv6 = m_device->ipV6Config().addresses().at(0).ip().toString();
        m_ipv6Sensor->setValue(ipv6);
        m_ipv6GatewaySensor->setValue(m_device->ipV6Config().gateway());
        m_ipv6SubnetMaskSensor->setValue(m_device->ipV6Config().addresses().at(0).netmask().toString());
        m_ipv6WithPrefixLengthSensor->setValue(static_cast<QString>(ipv6 + '/' + QString::number(m_device->ipV6Config().addresses().at(0).prefixLength())));
        auto dnsList = m_device->ipV6Config().nameservers();
        auto dnsListString = QString();
        if(!dnsList.isEmpty()) {
            dnsListString = std::accumulate(std::next(dnsList.begin()), dnsList.end(), dnsList.at(0).toString(),
                                            dnsAccumulationFunction);
        }
        m_ipv6DNSSensor->setValue(dnsListString);
    } else {
        m_ipv6Sensor->setValue(QString{});
        m_ipv6GatewaySensor->setValue(QString{});
        m_ipv6SubnetMaskSensor->setValue(QString{});
        m_ipv6WithPrefixLengthSensor->setValue(QString{});
        m_ipv6DNSSensor->setValue(QString{});
    }
}

bool NetworkManagerDevice::isConnected() const
{
    return m_connected;
}

void NetworkManagerDevice::updateWifi()
{
    if (!m_device->activeConnection()) {
        return;
    }

    auto activeConnectionName = m_wifiDevice->activeConnection()->connection()->name();
    const auto networks = m_wifiDevice->networks();
    std::for_each(networks.begin(), networks.end(), [this, activeConnectionName](QSharedPointer<NetworkManager::WirelessNetwork> network) {
        if (network->ssid() == activeConnectionName) {
            connect(network.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, m_signalSensor, &KSysGuard::SensorProperty::setValue, Qt::UniqueConnection);
            m_signalSensor->setValue(network->signalStrength());
        } else {
            network->disconnect(m_signalSensor);
        }
    });
}

NetworkManagerBackend::NetworkManagerBackend(QObject* parent)
    : NetworkBackend(parent)
{
}

NetworkManagerBackend::~NetworkManagerBackend()
{
    qDeleteAll(m_devices);
}

bool NetworkManagerBackend::isSupported()
{
    if (NetworkManager::status() == NetworkManager::Unknown) {
        return false;
    } else {
        return true;
    }
}

void NetworkManagerBackend::start()
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &NetworkManagerBackend::onDeviceAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &NetworkManagerBackend::onDeviceRemoved);

    const auto devices = NetworkManager::networkInterfaces();
    for (const auto &device : devices) {
        onDeviceAdded(device->uni());
    }
}

void NetworkManagerBackend::stop()
{
    NetworkManager::notifier()->disconnect(this);
}

void NetworkManagerBackend::onDeviceAdded(const QString& uni)
{
    auto device = NetworkManager::findNetworkInterface(uni);
    if (!device) {
        return;
    }

    switch (device->type()) {
        case NetworkManager::Device::Ethernet:
        case NetworkManager::Device::Wifi:
        case NetworkManager::Device::Bluetooth:
        case NetworkManager::Device::Modem:
        case NetworkManager::Device::Adsl:
            break;
        default:
            // Non-hardware devices, ignore them.
            return;
    }

    if (m_devices.contains(uni)) {
        return;
    }

    auto nmDevice = new NetworkManagerDevice(device->interfaceName(), device);
    connect(nmDevice, &NetworkManagerDevice::connected, this, &NetworkManagerBackend::deviceAdded);
    connect(nmDevice, &NetworkManagerDevice::disconnected, this, &NetworkManagerBackend::deviceRemoved);
    m_devices.insert(uni, nmDevice);

    if (nmDevice->isConnected()) {
        Q_EMIT deviceAdded(nmDevice);
    }
}

void NetworkManagerBackend::onDeviceRemoved(const QString& uni)
{
    if (!m_devices.contains(uni)) {
        return;
    }

    auto device = m_devices.take(uni);

    if (device->isConnected()) {
        Q_EMIT deviceRemoved(device);
    }

    delete device;
}


#include "moc_NetworkManagerBackend.cpp"

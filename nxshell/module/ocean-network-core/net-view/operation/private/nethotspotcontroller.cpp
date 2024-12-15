// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "nethotspotcontroller.h"

#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>
#include <hotspotcontroller.h>
#include <networkcontroller.h>
#include <wirelessdevice.h>

namespace ocean {
namespace network {
NetHotspotController::NetHotspotController(QObject *parent)
    : QObject(parent)
    , m_isEnabled(false)
    , m_enabledable(true)
    , m_updatedTimer(new QTimer(this))
{
    m_updatedTimer->setSingleShot(true);
    m_updatedTimer->setInterval(50);
    m_hotspotController = NetworkController::instance()->hotspotController();
    updateData();
    updateConfig();
    m_enabledable = m_hotspotController->supportHotspot();
    connect(m_hotspotController, &HotspotController::deviceAoceand, this, &NetHotspotController::updateData);
    connect(m_hotspotController, &HotspotController::deviceRemove, this, &NetHotspotController::updateData);
    connect(m_hotspotController, &HotspotController::activeConnectionChanged, this, &NetHotspotController::updateEnabled);
    connect(m_hotspotController, &HotspotController::activeConnectionChanged, this, &NetHotspotController::updateConfig);
    connect(m_hotspotController, &HotspotController::itemAoceand, this, &NetHotspotController::updateConfig);
    connect(m_hotspotController, &HotspotController::itemRemoved, this, &NetHotspotController::updateConfig);
    connect(m_hotspotController, &HotspotController::itemChanged, this, &NetHotspotController::updateConfig, Qt::QueuedConnection);
}

bool NetHotspotController::isEnabled() const
{
    return m_isEnabled;
}

bool NetHotspotController::enabledable() const
{
    return m_enabledable;
}

const QVariantMap &NetHotspotController::config() const
{
    return m_config;
}

const QStringList &NetHotspotController::optionalDevice() const
{
    return m_optionalDevice;
}

const QStringList &NetHotspotController::shareDevice() const
{
    return m_shareDevice;
}

void NetHotspotController::updateEnabled()
{
    bool enabled = false;
    for (auto dev : m_hotspotController->devices()) {
        enabled |= dev->hotspotEnabled();
        if (enabled) {
            break;
        }
    }
    if (enabled != m_isEnabled) {
        m_isEnabled = enabled;
        Q_EMIT enabledChanged(m_isEnabled);
    }
}

void NetHotspotController::updateEnabledable()
{
    bool enabledable = m_hotspotController->supportHotspot();
    if (enabledable != m_enabledable) {
        m_enabledable = enabledable;
        Q_EMIT enabledableChanged(m_enabledable);
    }
}

void NetHotspotController::updateData()
{
    QStringList optionalDevice;
    for (auto dev : m_hotspotController->devices()) {
        QString mac = dev->realHwAdr();
        if (mac.isEmpty()) {
            mac = dev->usingHwAdr();
        }
        mac = mac + " (" + dev->interface() + ")";
        optionalDevice.append(mac);
        connect(dev, &WirelessDevice::hotspotEnableChanged, this, &NetHotspotController::updateEnabled, Qt::UniqueConnection);
    }
    updateEnabled();
    updateEnabledable();

    if (optionalDevice != m_optionalDevice) {
        m_optionalDevice = optionalDevice;
        Q_EMIT optionalDeviceChanged(m_optionalDevice);
    }
}

void NetHotspotController::updateConfig()
{
    if (m_updatedTimer->isActive()) {
        return;
    }
    QList<HotspotItem *> items;
    for (auto dev : m_hotspotController->devices()) {
        items.append(m_hotspotController->items(dev));
    }
    NetworkManager::ConnectionSettings::Ptr settings;
    NetworkManager::Connection::Ptr conn;
    for (auto item : items) {
        auto connection = NetworkManager::findConnectionByUuid(item->connection()->uuid());
        if (!connection) {
            continue;
        }
        auto connectionSettings = connection->settings();
        if (settings.isNull()) {
            settings = connectionSettings;
            conn = connection;
        } else {
            if (connectionSettings->timestamp() > settings->timestamp()) {
                settings = connectionSettings;
                conn = connection;
            }
        }
    }
    if (settings.isNull()) {
        settings.reset(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));

        settings->setId(NetworkManager::hostname());
        settings->setAutoconnect(false);
        NetworkManager::WirelessSetting::Ptr wSetting = settings->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
        wSetting->setSsid(settings->id().toUtf8());
        wSetting->setMode(NetworkManager::WirelessSetting::Ap);
        wSetting->setSecurity("802-11-wireless-security");
        wSetting->setInitialized(true);
        NetworkManager::Ipv4Setting::Ptr ipv4Setting = settings->setting(NetworkManager::Setting::SettingType::Ipv4).staticCast<NetworkManager::Ipv4Setting>();
        ipv4Setting->setMethod(NetworkManager::Ipv4Setting::ConfigMethod::Shared);
        ipv4Setting->setInitialized(true);
    } else {
        // conn->secrets会触发两次itemChanged信号
        m_updatedTimer->start();
        NetworkManager::WirelessSecuritySetting::Ptr const sSetting = settings->setting(NetworkManager::Setting::SettingType::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
        sSetting->secretsFromMap(conn->secrets(sSetting->name()).value().value(sSetting->name()));
    }

    const NMVariantMapMap &settingsMap = settings->toMap();
    QVariantMap config;
    for (auto it = settingsMap.cbegin(); it != settingsMap.cend(); it++) {
        config.insert(it.key(), it.value());
    }
    QVariantMap typeMap = config["802-11-wireless"].value<QVariantMap>();
    typeMap.insert("optionalDevice", m_optionalDevice);
    config["802-11-wireless"] = typeMap;
    m_config = config;
    Q_EMIT configChanged(m_config);
}
} // namespace network
} // namespace ocean

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configwatcher.h"

#include <DConfig>

#include <QCoreApplication>
#include <QDebug>
#include <QGSettings>

static Dtk::Core::DConfig *dConfig = nullptr;
static QGSettings *gSettingsConfig = nullptr;
static QGSettings *gAirplaneConfig = nullptr;

ConfigWatcher::ConfigWatcher(QObject *parent)
    : QObject(parent)
    , m_networkAirplaneMode(false)
    , m_wpa3EnterpriseVisible(false)
    , m_wirelessScanInterval(0)
    , m_airplaneModeEnabled(false)
    , m_enableAccountNetwork(false)
{
    if (!dConfig)
        dConfig = Dtk::Core::DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.network");

    if (!gSettingsConfig && QGSettings::isSchemaInstalled("com.deepin.dde.control-center"))
        gSettingsConfig = new QGSettings("com.deepin.dde.control-center", QByteArray(), this);

    if (!gAirplaneConfig && QGSettings::isSchemaInstalled("com.deepin.dde.dock.module.airplane-mode"))
        gAirplaneConfig = new QGSettings("com.deepin.dde.dock.module.airplane-mode", QByteArray(), this);

    if (dConfig && dConfig->isValid()) {
        connect(dConfig, &Dtk::Core::DConfig::valueChanged, this, &ConfigWatcher::onValueChanged);

        QStringList keys = dConfig->keyList();
        if (keys.contains("networkAirplaneMode")) {
            m_networkAirplaneMode = dConfig->value("networkAirplaneMode").toBool();
        }
        if (keys.contains("lastProxyMethod")) {
            m_lastProxyMethod = dConfig->value("lastProxyMethod").toString();
        }
        if (keys.contains("WPA3-Enterprise-Visible")) {
            m_wpa3EnterpriseVisible = dConfig->value("WPA3-Enterprise-Visible").toBool();
        }
        if (keys.contains("wirelessScanInterval")) {
            m_wirelessScanInterval = dConfig->value("wirelessScanInterval", 10).toInt() * 1000;
        }
        if (keys.contains("enableAccountNetwork")) {
            m_enableAccountNetwork = dConfig->value("enableAccountNetwork").toBool();
        }
    }

    if (gSettingsConfig) {
        connect(gSettingsConfig, &QGSettings::changed, this, &ConfigWatcher::onDccConfigChanged);
        const QStringList keys = gSettingsConfig->keys();
        if (keys.contains("wireless"))
            m_wirelessState = gSettingsConfig->get("wireless").toString();
    }

    if (gAirplaneConfig) {
        connect(gAirplaneConfig, &QGSettings::changed, this, &ConfigWatcher::onAirplaneModeChanged);
        if (gAirplaneConfig->keys().contains("enable")) {
            m_airplaneModeEnabled = gAirplaneConfig->get("enable").toBool();
        }
    }
}

ConfigWatcher::~ConfigWatcher()
{
}

void ConfigWatcher::onValueChanged(const QString &key)
{
    if (key == "networkAirplaneMode") {
        m_networkAirplaneMode = dConfig->value("networkAirplaneMode").toBool();
        emit enableAirplaneModeChanged(m_networkAirplaneMode);
    } else if (key == "lastProxyMethod") {
        m_lastProxyMethod = dConfig->value("lastProxyMethod").toString();
        emit lastProxyMethodChanged(proxyMethod());
    } else if (key == "WPA3-Enterprise-Visible") {
        m_wpa3EnterpriseVisible = dConfig->value("WPA3-Enterprise-Visible").toBool();
        emit wpa3EnterpriseVisibleChanged(m_wpa3EnterpriseVisible);
    } else if (key == "wirelessScanInterval") {
        m_wirelessScanInterval = dConfig->value("wirelessScanInterval", 10).toInt() * 1000;
        emit wirelessScanIntervalChanged(m_wirelessScanInterval);
    }
}

void ConfigWatcher::onDccConfigChanged(const QString &key)
{
    if (key == "wireless") {
        ModuleState oldState = wirelessState();
        m_wirelessState = gSettingsConfig->get(key).toString();
        ModuleState newState = wirelessState();
        if (oldState != newState)
            emit wirelessStateChanged(newState);
    }
}

void ConfigWatcher::onAirplaneModeChanged(const QString &key)
{
    if (key == "enable") {
        bool enabled = gAirplaneConfig->get(key).toBool();
        if (enabled == m_airplaneModeEnabled)
            return;

        m_airplaneModeEnabled = enabled;
        emit airplaneModeEnabledChanged(m_airplaneModeEnabled);
    }
}

ConfigWatcher *ConfigWatcher::instance()
{
    static ConfigWatcher config;
    return &config;
}

bool ConfigWatcher::enableAirplaneMode() const
{
    return m_networkAirplaneMode;
}

bool ConfigWatcher::wpa3EnterpriseVisible() const
{
    return m_wpa3EnterpriseVisible;
}

dde::network::ProxyMethod ConfigWatcher::proxyMethod() const
{
    if (m_lastProxyMethod == "auto")
        return dde::network::ProxyMethod::Auto;

    return dde::network::ProxyMethod::Manual;
}

void ConfigWatcher::setProxyMethod(const dde::network::ProxyMethod &method)
{
    if (dConfig && dConfig->keyList().contains("lastProxyMethod")) {
        dConfig->setValue("lastProxyMethod", (method == dde::network::ProxyMethod::Auto ? "auto" : "manual"));
    }
}

ConfigWatcher::ModuleState ConfigWatcher::wirelessState() const
{
    if (m_wirelessState == "Hidden")
        return ModuleState::Hidden;

    if (m_wirelessState == "Enabled")
        return ModuleState::Enabled;

    if (m_wirelessState == "Disabled")
        return ModuleState::Disabled;

    return ModuleState::Enabled;
}

bool ConfigWatcher::airplaneModeEnabled() const
{
    return m_airplaneModeEnabled;
}

void ConfigWatcher::setAirplaneModeEnabled(bool enabled)
{
    m_airplaneModeEnabled = enabled;
    if (!gAirplaneConfig)
        return;

    gAirplaneConfig->set("enable", enabled);
}

int ConfigWatcher::wirelessScanInterval() const
{
    return m_wirelessScanInterval;
}

bool ConfigWatcher::enableAccountNetwork() const
{
    return m_enableAccountNetwork;
}

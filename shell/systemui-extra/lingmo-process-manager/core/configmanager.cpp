/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "configmanager.h"
#include "misc.h"
#include <QCoreApplication>
#include <QVariant>
#include <QDebug>
#include <sstream>
#include <fstream>
#include <iostream>

namespace {

const char *resource_limit_key = "resourceLimitEnabled";
const char *root_id = "org.lingmo.process-manager";
const char *default_apps_id = "org.lingmo.process-manager.default-group-apps";
const char *top_app_group_key = "topApp";
const char *whitelist_key = "whitelist";

const char *soft_freeze_mode_id = "org.lingmo.process-manager.soft-freeze-mode";
const char *soft_freeze_mode_enabled_key = "enabled";
const char *soft_freeze_mode_increased_battery_remaining_time = "increasedBatteryRemainingTime";

}

ConfigManager::ConfigManager()
    : m_resourceLimitEnabled(false)
    , m_preResourceLimitEnabled(false)
{
    initGSettings();
    loadGeneralConfig();
    loadPolicyConfig();
    initResourceLimitEnabledChangedTimer();
}

int ConfigManager::background2CachedStateInterval(sched_policy::DeviceMode mode) const
{
    std::string modeStr = sched_policy::deviceModeToString(mode);
    return m_background2CachedStateInterval[modeStr].asInt();
}

void ConfigManager::setReousrceLimitEnabled(bool enabled)
{
    m_rootGSettings->set(resource_limit_key, enabled);
}

void ConfigManager::setResourceLimitEnabledChangedCallback(ResourceLimitEnableChangedCallback callback)
{
    m_resouceLimitEnableChangeCallbacks.emplace_back(callback);
}

void ConfigManager::setSoftFreezeModeEnabledChangedCallback(ResourceLimitEnableChangedCallback callback)
{
    m_softFreezeModeEnabledChanedCallback = std::move(callback);
}

bool ConfigManager::addAppToWhitelist(const QString &desktopFile)
{
    auto whiteListDesktopFiles = appWhitelist();
    if (whiteListDesktopFiles.contains(desktopFile)) {
        return true;
    }

    whiteListDesktopFiles.append(desktopFile);
    return m_rootGSettings->trySet(whitelist_key, whiteListDesktopFiles);
}

bool ConfigManager::removeAppFromWhitelist(const QString &desktopFile)
{
    auto whiteListDesktopFiles = appWhitelist();
    if (!whiteListDesktopFiles.contains(desktopFile)) {
        return true;
    }

    whiteListDesktopFiles.removeOne(desktopFile);
    return m_rootGSettings->trySet(whitelist_key, whiteListDesktopFiles);
}

const QStringList &ConfigManager::appWhitelist() const
{
    return m_appWhitelist;
}

void ConfigManager::setIncreasedBatteryRemainingTime(int increasedTime)
{
    m_softFreezeModeGSettings->set(soft_freeze_mode_increased_battery_remaining_time, increasedTime);
}

void ConfigManager::initGSettings()
{
    initRootGSettings();
    initDefaultGroupAppGSettings();
    initSoftFreezeModeGsettings();
}

void ConfigManager::initRootGSettings()
{
    m_rootGSettings = doInitGSettings(
        root_id, [this](const std::shared_ptr<QGSettings> &gsettings, const QString &key) {
            if (key == resource_limit_key) {
                m_resourceLimitEnabled = m_rootGSettings->get(resource_limit_key).toBool();
                m_resourceLimitEnabledChangedTimer.start();
            } else if (key == whitelist_key) {
                m_appWhitelist = m_rootGSettings->get(whitelist_key).toStringList();
            }
        });

    if (m_rootGSettings) {
        m_resourceLimitEnabled = m_rootGSettings->get(resource_limit_key).toBool();
        m_appWhitelist = m_rootGSettings->get(whitelist_key).toStringList();
        m_preResourceLimitEnabled = m_resourceLimitEnabled;
    }
}

void ConfigManager::initDefaultGroupAppGSettings()
{
    m_defaultGroupAppsGSettings = doInitGSettings(
        default_apps_id, [this](const std::shared_ptr<QGSettings> &gsettings, const QString &key) {
            if (key == top_app_group_key) {
                m_defaultAppsInTopGroup = gsettings->get(top_app_group_key).toStringList();
            }
        });

    if (m_defaultGroupAppsGSettings) {
        m_defaultAppsInTopGroup = m_defaultGroupAppsGSettings->get(top_app_group_key).toStringList();
    }
}

void ConfigManager::initSoftFreezeModeGsettings()
{
    m_softFreezeModeGSettings = doInitGSettings(
        soft_freeze_mode_id, [this](const std::shared_ptr<QGSettings> &gsettings, const QString &key) {
            if (key == soft_freeze_mode_enabled_key) {
                m_softFreezeModeEnabled = gsettings->get(soft_freeze_mode_enabled_key).toBool();
                if (m_softFreezeModeEnabledChanedCallback) {
                    m_softFreezeModeEnabledChanedCallback(m_softFreezeModeEnabled);
                }
                qDebug() << "softFreezeModeEnabled " << m_softFreezeModeEnabled;
            }
        });

    if (m_softFreezeModeGSettings) {
        m_softFreezeModeEnabled = m_softFreezeModeGSettings->get(soft_freeze_mode_enabled_key).toBool();
    }
}

std::shared_ptr<QGSettings> ConfigManager::doInitGSettings(const QByteArray &schemaId,
                                                           GSettingsKeyChangedCallback keyChangedCallback)
{
    if (!QGSettings::isSchemaInstalled(schemaId)) {
        qWarning() << schemaId << " is not installed.";
        return nullptr;
    }

    std::shared_ptr<QGSettings> gsettings = std::make_shared<QGSettings>(schemaId);
    QObject::connect(gsettings.get(), &QGSettings::changed, [keyChangedCallback, gsettings] (const QString &key) {
        keyChangedCallback(gsettings, key);
    });

    return gsettings;
}

void ConfigManager::handleResourceLimitEnabledChanged()
{
    if (m_preResourceLimitEnabled == m_resourceLimitEnabled) {
        return;
    }

    handleResourceLimitEnabledChangedCallbacks();
    m_preResourceLimitEnabled = m_resourceLimitEnabled;
}

void ConfigManager::handleResourceLimitEnabledChangedCallbacks()
{
    for (const auto &callback : m_resouceLimitEnableChangeCallbacks) {
        if (callback == nullptr) {
            continue;
        }

        callback(m_resourceLimitEnabled);
    }
}

void ConfigManager::loadGeneralConfig()
{
    std::ifstream file(kGeneralConfigFile);
    if (!file.is_open()) {
        qWarning() << QString("Failed to open general config file: %1.").arg(QString::fromStdString(kGeneralConfigFile));
        return;
    }

    Json::Value root;
    file >> root;

    m_background2CachedStateInterval = root["Background2CachedSwitchInterval"];
}

void ConfigManager::loadPolicyConfig()
{
    const auto configFile = misc::version::cgroupVersion() == 2 ? kPolicyConfigFileV2 : kPolicyConfigFileV1;
    std::ifstream file(configFile);
    if (!file.is_open()) {
        qWarning() << QString("Failed to open policy config file: %1.").arg(QString::fromStdString(configFile));
        return;
    }
    m_policyConfigInfo.clear();
    file >> m_policyConfigInfo;
}

void ConfigManager::initResourceLimitEnabledChangedTimer()
{
    m_resourceLimitEnabledChangedTimer.setSingleShot(true);
    m_resourceLimitEnabledChangedTimer.setInterval(kResourceLimitEnabledChangedTimerInterval);
    QObject::connect(&m_resourceLimitEnabledChangedTimer, &QTimer::timeout, qApp, [this] {
        handleResourceLimitEnabledChanged();
    });
}

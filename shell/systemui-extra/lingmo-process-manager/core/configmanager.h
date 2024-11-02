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

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QTimer>
#include <string>
#include <functional>
#include <memory>
#include <list>
#include <jsoncpp/json/json.h>
#include <QGSettings/QGSettings>

#include "schedpolicy.h"

class ConfigManager
{
public:
    using ResourceLimitEnableChangedCallback = std::function<void(bool)>;

    ConfigManager();

    int background2CachedStateInterval(sched_policy::DeviceMode mode) const;
    bool reousrceLimitEnabled() const { return m_resourceLimitEnabled; }
    void setReousrceLimitEnabled(bool enabled);
    bool softFreezeModeEnabled() const { return m_softFreezeModeEnabled; }
    Json::Value policyConfigInfo() const { return m_policyConfigInfo; }
    QStringList defaultAppsInTopGroup() const { return m_defaultAppsInTopGroup; }
    
    void setResourceLimitEnabledChangedCallback(ResourceLimitEnableChangedCallback callback);
    void setSoftFreezeModeEnabledChangedCallback(ResourceLimitEnableChangedCallback callback);

    bool addAppToWhitelist(const QString &desktopFile);
    bool removeAppFromWhitelist(const QString &desktopFile);
    const QStringList &appWhitelist() const;

    void setIncreasedBatteryRemainingTime(int increasedTime);

private:
    using GSettingsKeyChangedCallback = std::function<void(const std::shared_ptr<QGSettings> &gsettings, const QString &key)>;

    void initGSettings();
    void initRootGSettings();
    void initDefaultGroupAppGSettings();
    void initSoftFreezeModeGsettings();
    std::shared_ptr<QGSettings> doInitGSettings(
        const QByteArray &schemaId, GSettingsKeyChangedCallback keyChangedCallback);
    void handleResourceLimitEnabledChangedCallbacks();
    void handleResourceLimitEnabledChanged();
    void loadGeneralConfig();
    void loadPolicyConfig();
    void initResourceLimitEnabledChangedTimer();

private:
    Json::Value m_background2CachedStateInterval;
    bool m_resourceLimitEnabled;
    bool m_preResourceLimitEnabled;
    bool m_softFreezeModeEnabled;
    QStringList m_defaultAppsInTopGroup;
    QStringList m_appWhitelist;
    Json::Value m_policyConfigInfo;
    std::shared_ptr<QGSettings> m_rootGSettings;
    std::shared_ptr<QGSettings> m_defaultGroupAppsGSettings;
    std::shared_ptr<QGSettings> m_appWhitelistGSettings;
    std::shared_ptr<QGSettings> m_softFreezeModeGSettings;
    QTimer m_resourceLimitEnabledChangedTimer;
    std::vector<ResourceLimitEnableChangedCallback> m_resouceLimitEnableChangeCallbacks;
    ResourceLimitEnableChangedCallback m_softFreezeModeEnabledChanedCallback;

    const int kResourceLimitEnabledChangedTimerInterval = 1000;
    const std::string kGeneralConfigFile = "/etc/lingmo-process-manager/lingmo-process-manager.json";
    const std::string kPolicyConfigFileV1 = "/etc/lingmo-process-manager/task-profiles-v1.json";
    const std::string kPolicyConfigFileV2 = "/etc/lingmo-process-manager/task-profiles-v2.json";
};

#endif // CONFIGMANAGER_H

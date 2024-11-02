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

#ifndef SCHEDPOLICYMANAGER_H
#define SCHEDPOLICYMANAGER_H

#include <memory>
#include <functional>
#include "groupmanager.h"
#include "configmanager.h"
#include "exceptiongroupprocesswatcher.h"
#include "multimediacontroller.h"
#include "appinfo.h"

class SchedPolicyManager
{
public:
    using GetAllAppInfoCallback = std::function<std::vector<AppInfo>()>;
    using AppGroupChangedCallback = std::function
        <void(const std::string &appId, const std::string &groupName)>;

    SchedPolicyManager(const std::shared_ptr<ConfigManager> &configManager,
                       const std::shared_ptr<ExceptionGroupProcessWatcher> &exceptionGroupWatcher,
                       sched_policy::PowerMode powerMode, sched_policy::DeviceMode deviceMode);

    void handleAppStarted(const AppInfo &appInfo);
    void handleAppStateChanged(const AppInfo &appInfo);
    void handleAppChildPidsUpdated(const AppInfo &appInfo);
    void handleMemoryWarning(int level);

    void setPowerMode(sched_policy::PowerMode mode);
    void setDeviceMode(sched_policy::DeviceMode mode);

    void setGetAllAppInfoCallback(GetAllAppInfoCallback callback);
    void setAppGroupChangedCallback(AppGroupChangedCallback callback);

private:
    void initGroupManager();
    void updateExceptionGroupWatcher();
    void setConfigManagerCallback();
    void createProcessGroup(const AppInfo &appInfo);
    void changeProcessGroupByState(const AppInfo &appInfo);
    void handleMultimediaWhenSavePowerModeIsSet();
    void handleMultimediaByState(const AppInfo &appInfo);
    bool shouldHandleAppStateChanged(const AppInfo &appInfo);
    bool isUnlimitedApp(const AppInfo &appInfo);
    bool isWhitelistApp(const AppInfo &appInfo);
    bool isPlayPauseMultimediaState(sched_policy::AppState state);
    sched_policy::GroupType getGroupTypeByAppInfo(const AppInfo &appInfo) const;
    sched_policy::GroupType getGroupTypeByNormalAppInfo(const AppInfo &appInfo) const;

    void handleResouceLimitEnabledChanged(bool enabled);
    void updateAllAppsGroup();
    void updateAppGroup(const AppInfo &appInfo);
    void moveAppToDefaultGroup(const AppInfo &appInfo);
    void moveAppToGroupByAppState(const AppInfo &appInfo);

    std::string generateAppScopeName(const std::string &appName) const;
    std::vector<AppInfo> getAllAppInfos();

    void reclaimCachedAppMemory();
    std::vector<std::string> getAllCachedAppGroups();

private:
    std::unique_ptr<GroupManager> m_groupManager;
    std::shared_ptr<ConfigManager> m_configManager;
    std::shared_ptr<ExceptionGroupProcessWatcher> m_exceptionGroupWatcher;
    MultimediaController m_multimediaController;
    GetAllAppInfoCallback m_getAllAppInfoCallback;
    AppGroupChangedCallback m_appGroupChangedCallback;
    sched_policy::PowerMode m_powerMode;
    sched_policy::DeviceMode m_deviceMode;
    std::mutex m_mutex;
};

#endif // SCHEDPOLICYMANAGER_H

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

#ifndef PROCESSINFOMANAGER_H
#define PROCESSINFOMANAGER_H

#include "appinfomanager.h"
#include "configmanager.h"

class ProcessInfoManager
{
public:
    using AppChangedCallback = AppInfoManager::AppChangedCallback;

    ProcessInfoManager(const std::shared_ptr<ConfigManager> &configManager);

    void handleWindowAdded(const std::string &wid);
    void handleWindowRemoved(const std::string &wid);
    void handleActiveWindowChanged(const std::string &wid);
    void handleWindowMinimized(const std::string &wid);
    void handleExceptionGroupSessionApp(const std::string &desktopFile, int pid);
    void handleMprisDbusAdded(int pid, const std::string &service);
    void handleStatusNotifierItemRegistered(int pid);

    void createAppInstance(
        const std::string &desktopFile,
        const std::vector<std::string> &args, int pid);
    void setAppGroupName(const std::string &appId, const std::string &groupName);

    void forceChangCachedStateTo(sched_policy::AppState state);
    bool forceChangeAppStateByPid(int pid, sched_policy::AppState state);

    void setBackgroundToCachedStateInterval(int interval);

    sched_policy::AppType getAppTypeByPid(int pid);
    std::string syncGetDesktopFileByPid(int pid);

    std::vector<AppInfo> getAllAppInfos();
    std::vector<AppInfo> getAppInfosWithState(sched_policy::AppState appState);

    AppInfo getLatestAppInfoByDesktopFile(
        const std::string &desktopFile, const std::vector<std::string> &args);
    AppInfo getLatestAppInfoByCmdline(const std::string &cmdline);
    AppInfo getAppInfo(const std::string &appId);

    std::string getDesktopFileByPid(int pid);

    void setAppStartedCallback(AppChangedCallback callback);
    void setAppStateChangedCallback(AppChangedCallback callback);
    void setAppChildPidsUpdatedCallback(AppChangedCallback callback);

private:
    AppInfoManager m_appInfoManager;
};

#endif // PROCESSINFOMANAGER_H

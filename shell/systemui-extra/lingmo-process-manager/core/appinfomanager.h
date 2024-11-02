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

#ifndef APPINFOMANAGER_H
#define APPINFOMANAGER_H

#include <QTimer>
#include <map>
#include <vector>
#include <string>
#include "appinfo.h"
#include "desktopfilemanager.h"
#include "configmanager.h"
#include "selection/selectionchecker.h"

class AppInfoManager
{
public:
    using AppChangedCallback = std::function<void(const AppInfo &)>;

    explicit AppInfoManager(const std::shared_ptr<ConfigManager> &configManager);

    void handleWindowAdded(const std::string &wid);
    void handleWindowRemoved(const std::string &wid);
    void handleActiveWindowChanged(const std::string &wid);
    void handleWindowMinimized(const std::string &wid);
    void handleExceptionGroupSessionApp(const std::string &desktopFile, int pid);
    void handleMprisDbusAdded(int pid, const std::string &service);
    void handleStatusItemNotifierItemRegistered(int pid);

    void createAppInstance(
        const std::string &desktopFile,
        const std::vector<std::string> &args, int pid);
    void setAppGroupName(const std::string &appId, const std::string &groupName);

    void setAppStartedCallback(AppChangedCallback callback);
    void setAppStateChangedCallback(AppChangedCallback callback);
    void setAppChildPidsUpdatedCallback(AppChangedCallback callback);

    bool forceChangeAppStateByPid(int pid, sched_policy::AppState state);
    void forceChangCachedStateTo(sched_policy::AppState state);

    sched_policy::AppType getAppTypeByDesktopFile(const std::string &desktopFile);
    sched_policy::AppType getAppTypeByPid(int pid);

    std::string syncGetDesktopFileByPid(int pid);

    std::vector<AppInfo> getAllAppInfos();
    std::vector<AppInfo> getAppInfosWithState(sched_policy::AppState appState);

    AppInfo getLatestAppInfoByDesktopFile(
        const std::string &desktopFile, const std::vector<std::string> &args);
    AppInfo getLatestAppInfoByCmdline(const std::string &cmdline);
    AppInfo getAppInfo(const std::string &appId);
    std::string getDesktopFileByPid(int pid);

    void setBackgroundToCachedStateInterval(int interval);

private:
    using AppInfoIterator = std::vector<AppInfo>::iterator;

    void initActiveWindowChangedTimer();
    void doHandleActiveWindowChanged();
    void addWidToAppInfo(AppInfoIterator appInfoIt, const std::string &wid);
    void updateAppCmdlineByWid(AppInfoIterator appInfoIt, const std::string &wid);
    void createAppInstanceWithWid(const std::string &wid);
    void appBecomeActiveWithWid(const std::string &wid);
    void appBecomeInactiveWithWid(const std::string &wid);

    void updateAppStateWithoutEmptyWindows(AppInfoIterator appInfoIt);
    void updateAppInfoWithEmptyWindows(AppInfoIterator appInfoIt);
    void setAppState(sched_policy::AppState appState, AppInfoIterator appInfoIt);
    unsigned long long setBackgroundStateTimer(const std::string &appId);
    bool canBeCachedState(AppInfoIterator appInfoIt) const;
    void resetBackgroundStateTimers();
    std::vector<std::string> appIdsWithTimer();

    AppInfo generateAppInfoByWid(const std::string &wid);
    AppInfo generateAppInfo(const std::string &desktopFile, int pid);

    using FindAppInfoPredicateCallback = std::function<bool(const AppInfo &)>;
    std::tuple<bool, AppInfoIterator> findAppInfo(const FindAppInfoPredicateCallback &predicateCallback);
    std::tuple<bool, AppInfoIterator> findAppInfoWithWid(const std::string &wid);
    std::tuple<bool, AppInfoIterator> findAppInfoWithPid(int pid);
    std::tuple<bool, AppInfoIterator> findAppInfoWithAppId(const std::string &appId);

    void addStateChangedTimerId(const std::string &appId, unsigned long long timerId);
    void deleteStateChangeTimerId(const std::string &appId);

    void saveMprisServiceToCache(int pid, const std::string &service);
    std::string takeMprisServiceFromCache(int pid);

    void saveSystemTrayIconAppToCache(int pid);
    int takeSystemTrayIconAppFromCache(int);

    bool canChangeState(const AppInfo &appInfo);

private:
    // <desktopfile id, appinfo list>
    std::map<std::string, std::vector<AppInfo>> m_appInfos;
    //<appId, timerId>
    std::map<std::string, unsigned long long> m_appStateChangeTimerIds;
    std::map<int, std::string> m_mprisServices;
    std::vector<int> m_systemTrayIconApps;
    DesktopFileManager m_desktopFileManager;
    std::shared_ptr<ConfigManager> m_configManager;

    std::string m_currentActiveWid;
    std::string m_preActiveWid;
    QTimer m_activeWindowChangedTimer;

    int m_backgroundToCachedStateInterval;

    SelectionChecker m_selectionChecker;

    AppChangedCallback m_appStartedCallback;
    AppChangedCallback m_appStateChangedCallback;
    AppChangedCallback m_appChildPidsUpdatedCallback;
};

#endif // APPINFOMANAGER_H

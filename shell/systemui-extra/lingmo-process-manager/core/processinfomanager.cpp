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

#include "processinfomanager.h"

ProcessInfoManager::ProcessInfoManager(const std::shared_ptr<ConfigManager> &configManager)
    : m_appInfoManager(configManager)
{

}

void ProcessInfoManager::handleWindowAdded(const std::string &wid)
{
    m_appInfoManager.handleWindowAdded(wid);
}

void ProcessInfoManager::handleWindowRemoved(const std::string &wid)
{
    m_appInfoManager.handleWindowRemoved(wid);
}

void ProcessInfoManager::handleActiveWindowChanged(const std::string &wid)
{
    m_appInfoManager.handleActiveWindowChanged(wid);
}

void ProcessInfoManager::handleWindowMinimized(const std::string &wid)
{
    m_appInfoManager.handleWindowMinimized(wid);
}

void ProcessInfoManager::handleExceptionGroupSessionApp(const std::string &desktopFile, int pid)
{
    m_appInfoManager.handleExceptionGroupSessionApp(desktopFile, pid);
}

void ProcessInfoManager::handleMprisDbusAdded(int pid, const std::string &service)
{
    m_appInfoManager.handleMprisDbusAdded(pid, service);
}

void ProcessInfoManager::handleStatusNotifierItemRegistered(int pid)
{
    m_appInfoManager.handleStatusItemNotifierItemRegistered(pid);
}

void ProcessInfoManager::createAppInstance(
    const std::string &desktopFile, const std::vector<std::string> &args, int pid)
{
    m_appInfoManager.createAppInstance(desktopFile, args, pid);
}

void ProcessInfoManager::setAppGroupName(const std::string &appId, const std::string &groupName)
{
    m_appInfoManager.setAppGroupName(appId, groupName);
}

void ProcessInfoManager::forceChangCachedStateTo(sched_policy::AppState state)
{
    m_appInfoManager.forceChangCachedStateTo(state);
}

bool ProcessInfoManager::forceChangeAppStateByPid(int pid, sched_policy::AppState state)
{
    return m_appInfoManager.forceChangeAppStateByPid(pid, state);
}

void ProcessInfoManager::setBackgroundToCachedStateInterval(int interval)
{
    m_appInfoManager.setBackgroundToCachedStateInterval(interval);
}

sched_policy::AppType ProcessInfoManager::getAppTypeByPid(int pid)
{
    return m_appInfoManager.getAppTypeByPid(pid);
}

std::string ProcessInfoManager::syncGetDesktopFileByPid(int pid)
{
    return m_appInfoManager.syncGetDesktopFileByPid(pid);
}

std::vector<AppInfo> ProcessInfoManager::getAllAppInfos()
{
    return m_appInfoManager.getAllAppInfos();
}

std::vector<AppInfo> ProcessInfoManager::getAppInfosWithState(sched_policy::AppState appState)
{
    return m_appInfoManager.getAppInfosWithState(appState);
}

AppInfo ProcessInfoManager::getLatestAppInfoByDesktopFile(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    return m_appInfoManager.getLatestAppInfoByDesktopFile(desktopFile, args);
}

AppInfo ProcessInfoManager::getLatestAppInfoByCmdline(const std::string &cmdline)
{
    return m_appInfoManager.getLatestAppInfoByCmdline(cmdline);
}

AppInfo ProcessInfoManager::getAppInfo(const std::string &appId)
{
    return m_appInfoManager.getAppInfo(appId);
}

std::string ProcessInfoManager::getDesktopFileByPid(int pid)
{
    return m_appInfoManager.getDesktopFileByPid(pid);
}

void ProcessInfoManager::setAppStartedCallback(AppChangedCallback callback)
{
    m_appInfoManager.setAppStartedCallback(std::move(callback));
}

void ProcessInfoManager::setAppStateChangedCallback(AppChangedCallback callback)
{
    m_appInfoManager.setAppStateChangedCallback(std::move(callback));
}

void ProcessInfoManager::setAppChildPidsUpdatedCallback(AppChangedCallback callback)
{
    m_appInfoManager.setAppChildPidsUpdatedCallback(std::move(callback));
}

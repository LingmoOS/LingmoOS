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

#include "schedpolicymanager.h"
#include <QUuid>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include "configmanager.h"
#include "exceptiongroupprocesswatcher.h"
#include "timewheel.h"
#include "singleton.h"
#include "appinfohelper.h"
#include "misc.h"

namespace {

const char *systemd_cgroup_path_v1 = "/sys/fs/cgroup/systemd";
const char *systemd_cgroup_path_v2 = "/sys/fs/cgroup";

// Only alphanum, ':' and '_' allowed in systemd unit names
QString escapeUnitName(const QString &input)
{
    QString res;
    const QByteArray bytes = input.toUtf8();
    for (const auto &c : bytes) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ':' || c == '_' || c == '.') {
            res += QLatin1Char(c);
        } else {
            res += QStringLiteral("\\x%1").arg(c, 2, 16, QLatin1Char('0'));
        }
    }

    return res;
}


}

SchedPolicyManager::SchedPolicyManager(const std::shared_ptr<ConfigManager> &configManager,
                                       const std::shared_ptr<ExceptionGroupProcessWatcher> &exceptionGroupWatcher,
                                       sched_policy::PowerMode powerMode, sched_policy::DeviceMode deviceMode)
    : m_groupManager(new GroupManager())
    , m_configManager(configManager)
    , m_exceptionGroupWatcher(exceptionGroupWatcher)
    , m_powerMode(powerMode)
    , m_deviceMode(deviceMode)
{
    initGroupManager();
    updateExceptionGroupWatcher();
    setConfigManagerCallback();

    m_groupManager->excutePolicy(sched_policy::generatePolicyId(m_deviceMode, m_powerMode));
}

void SchedPolicyManager::handleAppStarted(const AppInfo &appInfo)
{
    if (isWhitelistApp(appInfo)) {
        return;
    }

    createProcessGroup(appInfo);
}

void SchedPolicyManager::handleAppStateChanged(const AppInfo &appInfo)
{
    if (!shouldHandleAppStateChanged(appInfo)) {
        return;
    }

    handleMultimediaByState(appInfo);
    changeProcessGroupByState(appInfo);
}

void SchedPolicyManager::handleAppChildPidsUpdated(const AppInfo &appInfo)
{
    if (isWhitelistApp(appInfo)) {
        return;
    }

    createProcessGroup(appInfo);
}

void SchedPolicyManager::handleMemoryWarning(int level)
{
    Q_UNUSED(level);
    reclaimCachedAppMemory();
}

void SchedPolicyManager::setDeviceMode(sched_policy::DeviceMode mode)
{
    if (m_deviceMode == mode) {
        return;
    }

    m_deviceMode = mode;
    m_groupManager->excutePolicy(sched_policy::generatePolicyId(m_deviceMode, m_powerMode));
}

void SchedPolicyManager::setPowerMode(sched_policy::PowerMode mode)
{
    if (m_powerMode == mode) {
        return;
    }

    m_powerMode = mode;
    m_groupManager->excutePolicy(sched_policy::generatePolicyId(m_deviceMode, m_powerMode));

    if (mode == sched_policy::PowerMode::Save) {
        handleMultimediaWhenSavePowerModeIsSet();
    }
}

void SchedPolicyManager::setGetAllAppInfoCallback(GetAllAppInfoCallback callback)
{
    m_getAllAppInfoCallback = std::move(callback);
}

void SchedPolicyManager::setAppGroupChangedCallback(AppGroupChangedCallback callback)
{
    m_appGroupChangedCallback = std::move(callback);
}

void SchedPolicyManager::initGroupManager()
{
    m_groupManager->setGroupConfigs(m_configManager->policyConfigInfo());
}

void SchedPolicyManager::updateExceptionGroupWatcher()
{
    auto sessionScopePath = m_groupManager->groupPath(sched_policy::GroupType::SessionScopeGroup);
    if (sessionScopePath.empty()) {
        qWarning() << "session scope path is empty";
        return;
    }
    const std::string systemdCgroupRootPath = misc::version::cgroupVersion() == 1 ?
        systemd_cgroup_path_v1 : systemd_cgroup_path_v2;
    sessionScopePath = systemdCgroupRootPath + "/" + sessionScopePath;
    m_exceptionGroupWatcher->addPath(sessionScopePath, sched_policy::GroupType::SessionScopeGroup);
}

void SchedPolicyManager::setConfigManagerCallback()
{
    m_configManager->setResourceLimitEnabledChangedCallback([this](bool enabled) {
        handleResouceLimitEnabledChanged(enabled);
    });
}

void SchedPolicyManager::createProcessGroup(const AppInfo &appInfo)
{
    auto groupType = getGroupTypeByAppInfo(appInfo);
    const std::string groupName = generateAppScopeName(appInfo.name());
    const std::string groupPath =
        m_groupManager->createProcessGroup(groupName, appInfo.pids(), groupType);

    QTimer::singleShot(0, nullptr, [this, appInfo, groupPath]() {
        if (m_appGroupChangedCallback) {
            m_appGroupChangedCallback(appInfo.appId(), groupPath);
        }
    });
}

void SchedPolicyManager::changeProcessGroupByState(const AppInfo &appInfo)
{
    createProcessGroup(appInfo);
}

void SchedPolicyManager::handleMultimediaWhenSavePowerModeIsSet()
{
    auto appInfos = getAllAppInfos();
    for (const auto &appInfo : appInfos) {
        handleMultimediaByState(appInfo);
    }
}

void SchedPolicyManager::handleMultimediaByState(const AppInfo &appInfo)
{
    qDebug() << "call mpris dbus "
             << QString::fromStdString(appInfo.mprisDbusService())
             << (int)m_powerMode
             << isPlayPauseMultimediaState(appInfo.appState())
             << (int)appInfo.appState();
    if (appInfo.mprisDbusService().empty() ||
        !isPlayPauseMultimediaState(appInfo.appState())) {
        return;
    }

    if (m_powerMode == sched_policy::PowerMode::Save ||
        m_deviceMode == sched_policy::DeviceMode::Tablet) {
        m_multimediaController.pause(appInfo.mprisDbusService());
    }
}

bool SchedPolicyManager::shouldHandleAppStateChanged(const AppInfo &appInfo)
{
    return m_configManager->reousrceLimitEnabled() && !isWhitelistApp(appInfo) && !isUnlimitedApp(appInfo);
}

bool SchedPolicyManager::isUnlimitedApp(const AppInfo &appInfo)
{
    return appInfo.appType() == sched_policy::AppType::Session ||
           appInfo.appType() == sched_policy::AppType::Top;
}

bool SchedPolicyManager::isWhitelistApp(const AppInfo &appInfo)
{
    for (const auto &appDesktopFile : m_configManager->appWhitelist()) {
        if (appinfo_helper::isSameDesktopFile(appDesktopFile.toStdString(), appInfo.desktopFile())) {
            return true;
        }
    }
    return false;
}

bool SchedPolicyManager::isPlayPauseMultimediaState(sched_policy::AppState state)
{
     return state == sched_policy::AppState::Background ||
           state == sched_policy::AppState::Cached;
}

sched_policy::GroupType SchedPolicyManager::getGroupTypeByAppInfo(const AppInfo &appInfo) const
{
    switch (appInfo.appType()) {
    case sched_policy::AppType::Normal: {
        return getGroupTypeByNormalAppInfo(appInfo);
    }
    case sched_policy::AppType::Top:
        return sched_policy::GroupType::TopGroup;
    case sched_policy::AppType::Session: {
        return sched_policy::GroupType::SessionGroup;
    }
    default: {
        return sched_policy::GroupType::DefaultGroup;
    }
    }
}

sched_policy::GroupType SchedPolicyManager::getGroupTypeByNormalAppInfo(const AppInfo &appInfo) const
{
    if (m_configManager->reousrceLimitEnabled()) {
        switch (appInfo.appState()) {
        case sched_policy::AppState::Focus:
            return sched_policy::GroupType::FocusGroup;
        case sched_policy::AppState::Foreground:
            return sched_policy::GroupType::ForegroundGroup;
        case sched_policy::AppState::Background:
            return sched_policy::GroupType::BackgroundGroup;
        case sched_policy::AppState::Service:
            return sched_policy::GroupType::ServiceGroup;
        case sched_policy::AppState::Cached:
            return sched_policy::GroupType::CachedGroup;
        default:
            break;
        }
    }

    return sched_policy::GroupType::DefaultGroup;
}

void SchedPolicyManager::handleResouceLimitEnabledChanged(bool enabled)
{
    Q_UNUSED(enabled)
    updateAllAppsGroup();
}

void SchedPolicyManager::updateAllAppsGroup()
{
    auto appInfos = getAllAppInfos();
    for (const auto &appInfo : appInfos) {
        updateAppGroup(appInfo);
    }
}

void SchedPolicyManager::updateAppGroup(const AppInfo &appInfo)
{
    if (appInfo.pids().empty() || isUnlimitedApp(appInfo)) {
        return;
    }

    if (m_configManager->reousrceLimitEnabled()) {
        moveAppToGroupByAppState(appInfo);
    } else {
        moveAppToDefaultGroup(appInfo);
    }
}

void SchedPolicyManager::moveAppToDefaultGroup(const AppInfo &appInfo)
{
    m_groupManager->createProcessGroup(
        generateAppScopeName(appInfo.name()), appInfo.pids(), sched_policy::GroupType::DefaultGroup);
}

void SchedPolicyManager::moveAppToGroupByAppState(const AppInfo &appInfo)
{
    handleAppStateChanged(appInfo);
}

std::string SchedPolicyManager::generateAppScopeName(const std::string &appName) const
{
    const QString unitName = escapeUnitName(QString::fromStdString(appName));
    const QString name = QStringLiteral("app-%1-%2.scope").arg(
        unitName, QUuid::createUuid().toString(QUuid::Id128));
    return name.toStdString();
}

std::vector<AppInfo> SchedPolicyManager::getAllAppInfos()
{
    if (!m_getAllAppInfoCallback) {
        return std::vector<AppInfo> {};
    }
    auto future = std::async(std::launch::async, [this]() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_getAllAppInfoCallback();
    });

    std::future_status futureStatus;
    do {
        futureStatus = future.wait_for(std::chrono::milliseconds(1));
        qApp->processEvents();
    } while (futureStatus != std::future_status::ready);
    return future.get();
}

void SchedPolicyManager::reclaimCachedAppMemory()
{
    std::vector<std::string> cachedGroups = getAllCachedAppGroups();
    if (cachedGroups.empty())
        return;

    m_groupManager->freezeGroups(cachedGroups);
    m_groupManager->reclaimProcessGroups(cachedGroups);
}

std::vector<std::string> SchedPolicyManager::getAllCachedAppGroups()
{
    std::vector<AppInfo> apps = getAllAppInfos();
    std::vector<std::string> cachedGroups;

    for (const auto& app : apps) {
        if (app.appState() == sched_policy::AppState::Cached) {
            cachedGroups.emplace_back(std::move(app.groupName()));
        }
    }

    return cachedGroups;
}

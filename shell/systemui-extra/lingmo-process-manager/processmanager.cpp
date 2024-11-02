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

#include "processmanager.h"
#include <QObject>
#include <QApplication>
#include "energymeter/cpuenergymeter.h"
#include "systemenery/upowersystemenery.h"

ProcessManager::ProcessManager()
    : m_configManager(std::make_shared<ConfigManager>())
    , m_processInfoManager(std::make_shared<ProcessInfoManager>(m_configManager))
    , m_exceptionGroupWatcher(std::make_shared<ExceptionGroupProcessWatcher>(m_processInfoManager))
    , m_appWhitelist(new AppWhitelist(*m_configManager))
    , m_appWhitelistService(new AppWhitelistService(m_appWhitelist.get()))
    , m_appLaunchManager(new AppLaunchManager(*m_processInfoManager))
    , m_appLaunchManagerService(new AppLaunchManagerService(m_appLaunchManager.get()))
    , m_systemEnergy(new UPowerSystemEnery())
    , m_resourceWarningHandler(new ResourceWarningHandler(m_configManager, std::make_shared<SystemNotifiesSender>(m_configManager)))
{
    std::unique_ptr<PowerManager> powerManager(new PowerManager);
    std::unique_ptr<DeviceModeManager> deviceModeManager(
        new DeviceModeManager(m_configManager->softFreezeModeEnabled()));

    initProcessGroupConsumptionMonitor();
    initBatteryRemainingTimeEstimator();

    initSchedPolicyManager(powerManager->currentPowerMode(), deviceModeManager->currentDeviceMode());
    initEventWatcher(std::move(powerManager), std::move(deviceModeManager));

    setProcessInfoManagerCallbacks();
    initExceptionWatcherConnections();

    setResourceLimitEnableChangedCallback();
    startWatchers();

    registerWhitelistDbusService();
    registerAppLauncherDbusService();

    connectAppLauncherSignals();

    updateBattertRemainingTimeEstimatorState();

    setMemoryWarningCallback();

    setSoftFreezeModeEnabledChangedCallback();
}

ProcessManager::~ProcessManager()
{
    m_eventWatcher->stopWatcher();
    m_exceptionGroupWatcher->stopWatcher();
}

void ProcessManager::ThawProcess(int pid)
{
    if (!canFrozenProcess()) {
        return;
    }

    bool ret = m_processInfoManager->forceChangeAppStateByPid(pid, sched_policy::AppState::Focus);
    if (!ret) {
        sendErrorReply(QDBusError::InvalidArgs,
            QString("Can not find the application by the pid %1.").arg(pid));
    }
}

void ProcessManager::ThawFrozenProcesses()
{
    if (!canFrozenProcess()) {
        return;
    }

    m_processInfoManager->forceChangCachedStateTo(sched_policy::AppState::Background);
}

void ProcessManager::initSchedPolicyManager(
    sched_policy::PowerMode powerMode, sched_policy::DeviceMode deviceMode)
{
    m_schedPolicyManager.reset(new SchedPolicyManager(
        m_configManager, m_exceptionGroupWatcher, powerMode, deviceMode));

    m_schedPolicyManager->setGetAllAppInfoCallback([this]() {
        return m_processInfoManager->getAllAppInfos();
    });

    m_schedPolicyManager->setAppGroupChangedCallback(
        [this](const std::string &appId, const std::string &groupName) {
            m_processInfoManager->setAppGroupName(appId, groupName);
        });
}

void ProcessManager::initEventWatcher(
    std::unique_ptr<PowerManager> powerManager, std::unique_ptr<DeviceModeManager> deviceModeManager)
{
    m_eventWatcher.reset(
        new EventWatcher(std::move(powerManager), std::move(deviceModeManager)));

    m_eventWatcher->setWindowAddedCallback([this](const std::string &wid) {
        m_processInfoManager->handleWindowAdded(wid);
    });

    m_eventWatcher->setWindowRemovedCallback([this](const std::string &wid) {
        m_processInfoManager->handleWindowRemoved(wid);
    });

    m_eventWatcher->setActiveWindowChangedCallback([this](const std::string &wid) {
        m_processInfoManager->handleActiveWindowChanged(wid);
    });

    m_eventWatcher->setWindowMinimizedCallback([this](const std::string &wid) {
        m_processInfoManager->handleWindowMinimized(wid);
    });

    m_eventWatcher->setDeviceModeChangedCallback([this](sched_policy::DeviceMode mode) {
        m_schedPolicyManager->setDeviceMode(mode);
        m_appLaunchManager->setDeviceMode(mode);
        updateBattertRemainingTimeEstimatorState();
        m_processInfoManager->setBackgroundToCachedStateInterval(
            m_configManager->background2CachedStateInterval(mode));
    });

    m_eventWatcher->setPowerTypeChangedCallback([this](sched_policy::PowerType) {
        updateBattertRemainingTimeEstimatorState();
    });

    m_eventWatcher->setPowerModeChangedCallback([this](sched_policy::PowerMode mode) {
        m_schedPolicyManager->setPowerMode(mode);
    });

    m_eventWatcher->setMprisDbusAddedCallback([this](int pid, const std::string &service) {
        m_processInfoManager->handleMprisDbusAdded(pid, service);
    });

    m_eventWatcher->setStatusNotifierItemRegisteredCallback([this](int pid) {
        m_processInfoManager->handleStatusNotifierItemRegistered(pid);
    });

    m_eventWatcher->setResourceWarningCallback([this](const std::string& resource, int level) {
        m_resourceWarningHandler->handleResourceWarning(resource, level);
    });
}

void ProcessManager::initProcessGroupConsumptionMonitor()
{
    std::shared_ptr<EnergyMeter> cpuEnergyMeter(new CpuEnergyMeter());
    m_consumptionMonitor.reset(
        new ProcessGroupConsumptionMonitor(m_systemEnergy, cpuEnergyMeter, 60, 10));
}

void ProcessManager::initBatteryRemainingTimeEstimator()
{
    m_battertRemainingTimeEstimator.reset(
        new BatteryRemainingTimeEstimator(
            m_systemEnergy, m_consumptionMonitor, 60 * 10,
            [this]() {
                auto appInfos = m_processInfoManager->getAppInfosWithState(sched_policy::AppState::Cached);
                std::vector<std::string> appIds;
                for (const auto &appInfo : appInfos) {
                    appIds.emplace_back(appInfo.appId());
                }
                return appIds;
            }
    ));

    m_battertRemainingTimeEstimator->setIncreasedRemainTimeChangedCallback([this](int time) {
        qDebug() << "Remain time: " << time;
        m_configManager->setIncreasedBatteryRemainingTime(time);
    });
}

void ProcessManager::setProcessInfoManagerCallbacks()
{
    m_processInfoManager->setAppStartedCallback([this](const AppInfo &appInfo) {
        m_schedPolicyManager->handleAppStarted(appInfo);
        m_consumptionMonitor->addProcessGroup(appInfo.appId(), appInfo.pids(), [this](const std::string &appId) {
            AppInfo latestAppInfo = m_processInfoManager->getAppInfo(appId);
            return latestAppInfo.pids();
        });
    });

    m_processInfoManager->setAppStateChangedCallback([this](const AppInfo &appInfo) {
        m_schedPolicyManager->handleAppStateChanged(appInfo);
    });

    m_processInfoManager->setAppChildPidsUpdatedCallback([this](const AppInfo &appInfo) {
        m_schedPolicyManager->handleAppChildPidsUpdated(appInfo);
    });
}

void ProcessManager::initExceptionWatcherConnections()
{
    QObject::connect(m_exceptionGroupWatcher.get(), &ExceptionGroupProcessWatcher::exceptionSessionAppCaught,
                     qApp, [this](const std::string &desktopFile, int pid) {
                         m_processInfoManager->handleExceptionGroupSessionApp(desktopFile, pid);
    });
}

void ProcessManager::setResourceLimitEnableChangedCallback()
{
    m_configManager->setResourceLimitEnabledChangedCallback([this] (bool enabled) {
        updateBattertRemainingTimeEstimatorState();
        m_dataCollector.collectResouceLimitedEnebaledChanged(enabled);
    });
}

void ProcessManager::setSoftFreezeModeEnabledChangedCallback()
{
    m_configManager->setSoftFreezeModeEnabledChangedCallback([this] (bool enabled) {
        // 软冻模式开启时，自动打开分级冻结的功能
        if (enabled) {
            m_configManager->setReousrceLimitEnabled(true);
        }
        updateBattertRemainingTimeEstimatorState();
        m_dataCollector.collectSoftFreezeModeEnabledChanged(enabled);
        m_eventWatcher->handleSoftFreezeModeEnabledChanged(enabled);
    });
}

void ProcessManager::setMemoryWarningCallback()
{
    m_resourceWarningHandler->setMemoryWarningHanlder([this](int level) {
        m_schedPolicyManager->handleMemoryWarning(level);
    });
}

void ProcessManager::startWatchers()
{
    m_exceptionGroupWatcher->startWatcher();
    m_eventWatcher->startWatcher();
}

void ProcessManager::stopWatchers()
{
    m_exceptionGroupWatcher->stopWatcher();
    m_eventWatcher->stopWatcher();
}

void ProcessManager::registerWhitelistDbusService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.lingmo.ProcessManager") ||
        !connection.registerObject("/com/lingmo/ProcessManager/AppWhitelist", m_appWhitelist.get())) {
        qWarning() << "Register whitelist dbus service failed:" << connection.lastError();
    }
}

void ProcessManager::registerAppLauncherDbusService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.lingmo.ProcessManager") ||
        !connection.registerObject("/com/lingmo/ProcessManager/AppLauncher", m_appLaunchManager.get())) {
        qWarning() << "Register app launcher dbus service failed:" << connection.lastError();
    }
}

void ProcessManager::connectAppLauncherSignals()
{
    QObject::connect(
        m_appLaunchManager.get(), &AppLaunchManager::AppLaunched,
        m_appLaunchManagerService.get(), &AppLaunchManagerService::AppLaunched);
}

void ProcessManager::updateBattertRemainingTimeEstimatorState()
{
    if (m_eventWatcher->powerType() == sched_policy::PowerType::Battery &&
        m_eventWatcher->deviceMode() == sched_policy::DeviceMode::PC &&
        !m_configManager->softFreezeModeEnabled() &&
        m_configManager->reousrceLimitEnabled()) {
       m_battertRemainingTimeEstimator->startEstimate();
    } else {
        m_battertRemainingTimeEstimator->stopEstimate();
    }
}

bool ProcessManager::canFrozenProcess()
{
    if (!m_configManager->reousrceLimitEnabled()) {
        return false;
    }

    if (m_eventWatcher->deviceMode() == sched_policy::DeviceMode::PC &&
        !m_configManager->softFreezeModeEnabled()) {
        return false;
    }

    return true;
}

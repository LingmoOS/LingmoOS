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

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <memory>
#include "processinfomanager.h"
#include "configmanager.h"
#include "eventwatcher.h"
#include "schedpolicymanager.h"
#include "exceptiongroupprocesswatcher.h"
#include "appwhitelist.h"
#include "appwhitelistservice.h"
#include "applaunchmanager.h"
#include "applaunchmanagerservice.h"
#include "consumptionmonitor/processgroupconsumptionmonitor.h"
#include "batteryremainingtimeestimator/batteryremainingtimeestimator.h"
#include "datacollector.h"
#include "resourcewarninghandler.h"

class ProcessManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    ProcessManager();
    ~ProcessManager();

    void ThawProcess(int pid);
    void ThawFrozenProcesses();

private:
    void initSchedPolicyManager(sched_policy::PowerMode powerMode, sched_policy::DeviceMode deviceMode);
    void initEventWatcher(std::unique_ptr<PowerManager> powerManager, std::unique_ptr<DeviceModeManager> deviceModeManager);
    void initEventWatcherAndSchedPolicyManager();
    void initProcessGroupConsumptionMonitor();
    void initBatteryRemainingTimeEstimator();
    void setEventWatcherCallbacks();
    void setProcessInfoManagerCallbacks();
    void initExceptionWatcherConnections();
    void setResourceLimitEnableChangedCallback();
    void setSoftFreezeModeEnabledChangedCallback();
    void setMemoryWarningCallback();
    void startWatchers();
    void stopWatchers();
    void registerWhitelistDbusService();
    void registerAppLauncherDbusService();
    void connectAppLauncherSignals();
    void updateBattertRemainingTimeEstimatorState();
    bool canFrozenProcess();

private:
    std::shared_ptr<ConfigManager> m_configManager;
    std::shared_ptr<ProcessInfoManager> m_processInfoManager;
    std::shared_ptr<ExceptionGroupProcessWatcher> m_exceptionGroupWatcher;
    std::unique_ptr<EventWatcher> m_eventWatcher;
    std::unique_ptr<SchedPolicyManager> m_schedPolicyManager;
    std::unique_ptr<AppWhitelist> m_appWhitelist;
    std::unique_ptr<AppWhitelistService> m_appWhitelistService;
    std::unique_ptr<AppLaunchManager> m_appLaunchManager;
    std::unique_ptr<AppLaunchManagerService> m_appLaunchManagerService;
    std::shared_ptr<ProcessGroupConsumptionMonitor> m_consumptionMonitor;
    std::shared_ptr<SystemEnergy> m_systemEnergy;
    std::unique_ptr<BatteryRemainingTimeEstimator> m_battertRemainingTimeEstimator;
    DataCollector m_dataCollector;
    std::unique_ptr<ResourceWarningHandler> m_resourceWarningHandler;
};

#endif // PROCESSMANAGER_H

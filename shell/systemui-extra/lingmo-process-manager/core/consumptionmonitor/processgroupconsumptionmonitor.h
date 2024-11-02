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

#ifndef PROCESSGROUPCONSUMPTIONMONITOR_H
#define PROCESSGROUPCONSUMPTIONMONITOR_H

#include <QTimer>
#include <list>
#include <string>
#include <functional>
#include <memory>
#include "core/energymeter/energymeter.h"
#include "core/systemenery/systemenergy.h"

class ProcessGroupConsumptionMonitor
{
public:
    using UpdatePidsCallback = std::function<std::vector<int>(const std::string &id)>; 

    ProcessGroupConsumptionMonitor(
        std::shared_ptr<SystemEnergy> systemEnergy,
        std::shared_ptr<EnergyMeter> cpuEnergyMeter,
        int monitorInterval, int retainedDataNumber);

    void addProcessGroup(
        const std::string &id, const std::vector<int> &pids,
        UpdatePidsCallback updatePidsCallback);

    double cpuEnergyConsumptionRate(const std::string &id) const;

private:
    struct ProcessGroup {
        std::string id;
        std::vector<int> pids;
        UpdatePidsCallback pidsGetter;
        // 当前监测周期内的监测间隔，第一次进行监测时大概率不是整个监测周期
        int monitorInterval;
        unsigned long long preCpuTimeConsumption;
        unsigned long long curCpuTimeConsumption;
        std::list<double> cpuEnergyConsumptionRates;
        void updateProcessGroupCpuTimeConsumption();
        double calculateCpuEnergyConsumptionRate() const;
    };

private:
    void initProcessGroup();

    ProcessGroup createProcessGroup(
        const std::string &id, const std::vector<int> &pids,
        UpdatePidsCallback &&updatePidsCallback);

    bool isMonitorRunning() const;
    void startMonitor();
    void stopMonitor();
    int monitorRemainingTime() const;
    void updateCpuConsumption();
    void updateTotalCpuConsumption();
    void updateAllProcessGroupCpuConsumption();
    void updateProcessGroupCpuConsumption(ProcessGroup &processGroup) const;
    void updateProcessGroupCpuTimeConsumption(ProcessGroup &processGroup) const;
    double calculateProcessGroupCpuTimeRatio(const ProcessGroup &processGroup) const;
    double calculateProcessGroupCpuConsumptionRate(const ProcessGroup &processGroup) const;

private:
    std::shared_ptr<SystemEnergy> m_systemEnergy;
    std::shared_ptr<EnergyMeter> m_totalCpuEnergyMeter;
    // seconds
    int m_monitorInterval;
    int m_retainedDataNumber;
    QTimer m_timer;
    unsigned long long m_preTotalCpuTimeConsumption;
    unsigned long long m_curTotalCpuTimeConsumption;
    std::map<std::string, ProcessGroup> m_processesCpuConsumption;
};

#endif // PROCESSGROUPCONSUMPTIONMONITOR_H

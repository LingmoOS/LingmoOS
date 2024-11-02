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

#include "processgroupconsumptionmonitor.h"

namespace {

int readProcStat(const char *interface, char *buffer, int bufferSize)
{
    FILE* file = fopen(interface, "r");
    if (file == nullptr) {
        perror("Could not open stat file");
        return -1;
    }

    char *ret = fgets(buffer, bufferSize, file);
    if (ret == nullptr) {
        perror("Could not read stat file");
        fclose(file);
        return -1;
    }
    fclose(file);

    return 0;
}

unsigned long long getTotalCpuTime()
{
    unsigned long long user = 0, nice = 0, system = 0;
    char buffer[1024];
    int ret = readProcStat("/proc/stat", buffer, sizeof(buffer) - 1);
    if (ret != 0) {
        return 0;
    }

    sscanf(buffer, "cpu  %16llu %16llu %16llu", &user, &nice, &system);
    return user + nice + system;
}

unsigned long long getProcessCpuTime(int pid)
{
    char procInterface[1024];
    sprintf(procInterface, "/proc/%d/stat", pid);
    char buffer[1024];
    int ret = readProcStat(procInterface, buffer, sizeof(buffer) - 1);
    if (ret != 0) {
        return 0;
    }

    unsigned long utime = 0, stime = 0;
    unsigned long int cutime = 0, cstime = 0;
    // pid(%d) comm(%s) state(%c) ppid(%d) pgrp(%d) session(%d) tty_nr(%d) 
    // tpgid(%d) flags(%u) minflt(%lu) cminflt(%lu) majflt(%lu) cmajflt(%lu) 
    // utime(%lu) stime(%lu) cutime(%ld) cstime(%ld)
    sscanf(buffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*16u %*16u %*16u %*16u %16lu %16lu %16ld %16ld",
           &utime, &stime, &cutime, &cstime);

    return utime + stime + cutime + cstime;
}

}

ProcessGroupConsumptionMonitor::ProcessGroupConsumptionMonitor(
    std::shared_ptr<SystemEnergy> systemEnergy,
    std::shared_ptr<EnergyMeter> cpuEnergyMeter,
    int monitorInterval, int retainedDataNumber)
    : m_systemEnergy(systemEnergy)
    , m_totalCpuEnergyMeter(cpuEnergyMeter)
    , m_monitorInterval(monitorInterval)
    , m_retainedDataNumber(retainedDataNumber)
{
    m_timer.setInterval(m_monitorInterval * 1000);
    QObject::connect(&m_timer, &QTimer::timeout, [this](){
        updateCpuConsumption();
    });
}

void ProcessGroupConsumptionMonitor::addProcessGroup(
    const std::string &id, const std::vector<int> &pids, UpdatePidsCallback updatePidsCallback)
{
    if (id.empty() || pids.empty()) {
        return;
    }

    ProcessGroup processGroup = 
        std::move(createProcessGroup(id, pids, std::move(updatePidsCallback)));

    m_processesCpuConsumption[id] = processGroup;

    if (!isMonitorRunning()) {
        startMonitor();
    }
}

double ProcessGroupConsumptionMonitor::cpuEnergyConsumptionRate(const std::string &id) const
{
    if (m_processesCpuConsumption.count(id) == 0) {
        return 0.0;
    }

    const ProcessGroup &processGroup = m_processesCpuConsumption.at(id);
    return processGroup.calculateCpuEnergyConsumptionRate();
}

ProcessGroupConsumptionMonitor::ProcessGroup ProcessGroupConsumptionMonitor::createProcessGroup(
    const std::string &id, const std::vector<int> &pids, UpdatePidsCallback &&updatePidsCallback)
{
    unsigned long long curProcessCpuTotalTime = 0;
    for (const auto &pid: pids) {
        curProcessCpuTotalTime += getProcessCpuTime(pid);
    }

    return ProcessGroup {
            id, pids, std::move(updatePidsCallback), monitorRemainingTime(),
            0, curProcessCpuTotalTime, {} 
        };
}

bool ProcessGroupConsumptionMonitor::isMonitorRunning() const
{
    return m_timer.isActive();
}

void ProcessGroupConsumptionMonitor::startMonitor()
{
    if (isMonitorRunning()) {
        return;
    }

    m_timer.start();
    m_totalCpuEnergyMeter->startMeasurement();
    m_curTotalCpuTimeConsumption = getTotalCpuTime();
}

void ProcessGroupConsumptionMonitor::stopMonitor()
{
    m_timer.stop();
}

int ProcessGroupConsumptionMonitor::monitorRemainingTime() const
{
    if (isMonitorRunning()) {
        return m_timer.remainingTime();
    }

    return m_timer.interval();
}

void ProcessGroupConsumptionMonitor::updateCpuConsumption()
{
    m_totalCpuEnergyMeter->endMeasurement();

    updateTotalCpuConsumption();
    updateAllProcessGroupCpuConsumption();

    if (m_processesCpuConsumption.empty()) {
        stopMonitor();
        return;
    }

    m_totalCpuEnergyMeter->startMeasurement();
}

void ProcessGroupConsumptionMonitor::updateTotalCpuConsumption()
{
    m_preTotalCpuTimeConsumption = m_curTotalCpuTimeConsumption;
    m_curTotalCpuTimeConsumption = getTotalCpuTime();
}

void ProcessGroupConsumptionMonitor::updateAllProcessGroupCpuConsumption()
{
    for (auto it = m_processesCpuConsumption.begin();
         it != m_processesCpuConsumption.end();) {

        auto pids = it->second.pidsGetter(it->first);
        if (pids.empty()) {
            m_processesCpuConsumption.erase(it ++);
            continue;
        }

        it->second.pids = pids;
        updateProcessGroupCpuConsumption(it->second);

        ++ it;
    }
}

void ProcessGroupConsumptionMonitor::updateProcessGroupCpuConsumption(ProcessGroup &processGroup) const
{
    processGroup.updateProcessGroupCpuTimeConsumption();
    double processGroupConsumptionRate = calculateProcessGroupCpuConsumptionRate(processGroup);
    processGroup.cpuEnergyConsumptionRates.emplace_back(processGroupConsumptionRate);

    if (processGroup.cpuEnergyConsumptionRates.size() > m_retainedDataNumber) {
        processGroup.cpuEnergyConsumptionRates.pop_front();
    }

    processGroup.monitorInterval = m_timer.interval();
}

void ProcessGroupConsumptionMonitor::updateProcessGroupCpuTimeConsumption(ProcessGroup &processGroup) const
{
    processGroup.preCpuTimeConsumption = processGroup.curCpuTimeConsumption;
    processGroup.curCpuTimeConsumption = 0;
    for (const auto &pid: processGroup.pids) {
        processGroup.curCpuTimeConsumption += getProcessCpuTime(pid);
    }
}

double ProcessGroupConsumptionMonitor::calculateProcessGroupCpuTimeRatio(const ProcessGroup &processGroup) const
{
    unsigned long long processCpuTimeDiff = processGroup.curCpuTimeConsumption - processGroup.preCpuTimeConsumption;
    unsigned long long totalCpuTimeDiff = m_curTotalCpuTimeConsumption - m_preTotalCpuTimeConsumption;
    double processMonitorIntervalRatio = (double)processGroup.monitorInterval / m_timer.interval();

    return ((double)processCpuTimeDiff / totalCpuTimeDiff) * processMonitorIntervalRatio;
}

double ProcessGroupConsumptionMonitor::calculateProcessGroupCpuConsumptionRate(const ProcessGroup &processGroup) const
{
    double processGroupCpuConsumptionRatio = calculateProcessGroupCpuTimeRatio(processGroup);
    double totalCpuEnergyConsumptionRate = m_totalCpuEnergyMeter->energyConsumptionRate();
    return processGroupCpuConsumptionRatio * totalCpuEnergyConsumptionRate;
}

void ProcessGroupConsumptionMonitor::ProcessGroup::updateProcessGroupCpuTimeConsumption()
{
    preCpuTimeConsumption = curCpuTimeConsumption;
    curCpuTimeConsumption = 0;
    for (const auto &pid: pids) {
        curCpuTimeConsumption += getProcessCpuTime(pid);
    }
}

double ProcessGroupConsumptionMonitor::ProcessGroup::calculateCpuEnergyConsumptionRate() const
{
    if (cpuEnergyConsumptionRates.empty()) {
        return 0.0;
    }

    double rateSum = std::accumulate(
        cpuEnergyConsumptionRates.begin(), cpuEnergyConsumptionRates.end(), 0.0);

    return rateSum / cpuEnergyConsumptionRates.size();
}

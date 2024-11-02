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

#include "batteryremainingtimeestimator.h"
#include <QDebug>

BatteryRemainingTimeEstimator::BatteryRemainingTimeEstimator(
    std::shared_ptr<SystemEnergy> systemEnergy,
    std::shared_ptr<ProcessGroupConsumptionMonitor> processGroupConsumptionMonitor,
    int estimateInterval,
    FrozenProcessGroupIdsGetter frozenProcessGroupIdsGetter)
    : m_systemEnergy(systemEnergy)
    , m_processGroupConsumptionMonitor(processGroupConsumptionMonitor)
    , m_estimateInterval(estimateInterval)
    , m_frozenProcessGroupIdsGetter(std::move(frozenProcessGroupIdsGetter))
{
    QObject::connect(&m_timer, &QTimer::timeout, [this]() {
        estimateRemainingTime();
    });
    m_timer.setInterval(m_estimateInterval * 1000);
}

void BatteryRemainingTimeEstimator::setIncreasedRemainTimeChangedCallback(
    RemainingTimeChangedCallback callback)
{
    m_increasedRemainingTimeChangedCallback = std::move(callback);
}

void BatteryRemainingTimeEstimator::startEstimate()
{
    qDebug() << "Battery remaning time estimator start.";
    if (!m_timer.isActive()) {
        m_timer.start();
        estimateRemainingTime();    
    }
}

void BatteryRemainingTimeEstimator::stopEstimate()
{
    qDebug() << "Battery remaning time estimator stop.";
    m_timer.stop();
    setIncreasedRemainingTime(0);
}

void BatteryRemainingTimeEstimator::estimateRemainingTime()
{
    std::vector<std::string> frozenGroupIds = m_frozenProcessGroupIdsGetter();
    double frozenGroupRate = calculateFrozenGroupsEnergyRate(frozenGroupIds);
    double estimatedEnergyRate = m_systemEnergy->dischargeRate() - frozenGroupRate;

    int increasedTime = 0;
    if (estimatedEnergyRate > 0.0001f) {
        double currentEnergy = m_systemEnergy->energy();
        int batteryRemainingTime = currentEnergy / estimatedEnergyRate;
        increasedTime = batteryRemainingTime > m_systemEnergy->timeToEmpty() ?
            batteryRemainingTime - m_systemEnergy->timeToEmpty() : 0;
    }

    if (increasedTime < 60) {
        increasedTime = m_systemEnergy->timeToEmpty() * 0.1;
    }

    setIncreasedRemainingTime(increasedTime);
}

double BatteryRemainingTimeEstimator::calculateFrozenGroupsEnergyRate(
    const std::vector<std::string> &groupIds)
{
    double energyRate = 0.0;
    for (const auto &id : groupIds) {
        energyRate += m_processGroupConsumptionMonitor->cpuEnergyConsumptionRate(id);
    }

    return energyRate;
}

void BatteryRemainingTimeEstimator::setIncreasedRemainingTime(int time)
{
    if (m_increasedRemainingTimeChangedCallback) {
        m_increasedRemainingTimeChangedCallback(time);
    }
}

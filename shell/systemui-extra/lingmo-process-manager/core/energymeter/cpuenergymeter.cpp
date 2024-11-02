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

#include "cpuenergymeter.h"

CpuEnergyMeter::CpuEnergyMeter()
    : m_startEnergy{0}
    , m_endEnergy{0}
{
}

void CpuEnergyMeter::startMeasurement()
{
    m_startTimePoint = std::chrono::high_resolution_clock::now();
    m_startEnergy = m_dbusInterface.getCpuEnergyConsumption();
}

void CpuEnergyMeter::endMeasurement()
{
    m_endTimePoint = std::chrono::high_resolution_clock::now();
    m_endEnergy = m_dbusInterface.getCpuEnergyConsumption();
}

double CpuEnergyMeter::energyConsumption() const
{
    return m_endEnergy - m_startEnergy;
}

double CpuEnergyMeter::energyConsumptionRate() const
{
    if (m_endTimePoint <= m_startTimePoint) {
        return 0;
    }

    auto deltaTime = std::chrono::duration_cast
        <std::chrono::milliseconds>(m_endTimePoint - m_startTimePoint);

    // 计算功率，转换成瓦
    return energyConsumption() / (deltaTime.count() / 1000);
}

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

#ifndef CPUENERGYMETER_H
#define CPUENERGYMETER_H

#include <chrono>
#include "energymeter.h"
#include "base/daemondbusinterface.h"

class CpuEnergyMeter : public EnergyMeter
{
public:
    CpuEnergyMeter();
    ~CpuEnergyMeter() = default;

    void startMeasurement() override;
    void endMeasurement() override;
    double energyConsumption() const override;
    double energyConsumptionRate() const override;

private:
    DaemonDbusInterface m_dbusInterface;
    //
    double m_startEnergy;
    double m_endEnergy;
    std::chrono::high_resolution_clock::time_point m_startTimePoint;
    std::chrono::high_resolution_clock::time_point m_endTimePoint;
};

#endif // CPUENERGYMETER_H

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

#ifndef CPURAPLENERGYMETER_H
#define CPURAPLENERGYMETER_H

#include <string>

class CpuRaplEnergyMeter
{
public:
    CpuRaplEnergyMeter();

    double energyConsumption() const;
    bool isAvailable() const;

private:
    bool checkIfPowercapSupported() const;
    bool checkIfMsrSupported() const;
    std::string getMsrInterface() const;

    double readEnergyFromPowercap() const;
    double readEnergyFromMsr() const;
    double readRaplPowerUnit() const;
    int readPowercap(const std::string &interface, double *value) const;
    int readMsr(const std::string &interface, int offset, uint64_t *value) const;

private:
    bool m_isPowercapSupported;
    bool m_isMsrSupported;
};

#endif // CPURAPLENERGYMETER_H

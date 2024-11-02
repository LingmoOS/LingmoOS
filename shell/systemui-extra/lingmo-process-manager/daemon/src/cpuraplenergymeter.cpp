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

#include "cpuraplenergymeter.h"
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <math.h>
#include <QDebug>

namespace {

const char *rapl_powercap_energy_interface =
    "/sys/devices/virtual/powercap/intel-rapl/intel-rapl:0/energy_uj";

const char *rapl_msr_energy_interface1 = "/dev/cpu/0/msr";
const char *rapl_msr_energy_interface2 = "/dev/cpu/msr0";

const int msr_power_unit_offset = 0x606;
const int msr_package_energy_offset = 0x611;

}

CpuRaplEnergyMeter::CpuRaplEnergyMeter()
{
    m_isPowercapSupported = checkIfPowercapSupported();
    m_isMsrSupported = checkIfMsrSupported();
}

double CpuRaplEnergyMeter::energyConsumption() const
{
    if (m_isPowercapSupported) {
        return readEnergyFromPowercap();
    }

    if (m_isMsrSupported) {
        return readEnergyFromMsr();
    }

    return 0;
}

bool CpuRaplEnergyMeter::isAvailable() const
{
    return m_isPowercapSupported || m_isMsrSupported;
}

bool CpuRaplEnergyMeter::checkIfPowercapSupported() const
{
    return access(rapl_powercap_energy_interface, R_OK) == 0;
}

bool CpuRaplEnergyMeter::checkIfMsrSupported() const
{
    return !getMsrInterface().empty();
}

int CpuRaplEnergyMeter::readMsr(const std::string &interface, int offset, uint64_t *value) const
{
    if (interface.empty()) {
        return -1;
    }

    int fd = open(interface.c_str(), O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    uint64_t msrValue;
    ssize_t retval = pread(fd, &msrValue, sizeof(msrValue), offset);
    close(fd);

    if (retval != sizeof(msrValue)) {
        return -1;
    }

    *value = msrValue;

    return 0;
}

std::string CpuRaplEnergyMeter::getMsrInterface() const
{
    if (access(rapl_msr_energy_interface1, R_OK) == 0) {
        return rapl_msr_energy_interface1;
    }

    if (access(rapl_msr_energy_interface2, R_OK) == 0) {
        return rapl_msr_energy_interface2;
    }

    return std::string();
}

double CpuRaplEnergyMeter::readEnergyFromPowercap() const
{
    double energy;
    int ret = readPowercap(rapl_powercap_energy_interface, &energy);
    if (ret < 0) {
        qWarning() << "Read energy from powercap failed";
        return 0;
    }
    // 微焦耳转成焦耳
    return energy / 1000000;
}

int CpuRaplEnergyMeter::readPowercap(const std::string &interface, double *value) const
{
    std::ifstream raplInterface(interface, std::ios::in);
    if (!raplInterface.is_open()) {
        qWarning() << "Failed to read powercap interface.";
        return -1;
    }

    std::string energyUj;
    std::getline(raplInterface, energyUj);
    *value = std::atof(energyUj.c_str());
    raplInterface.close();

    return 0;
}

double CpuRaplEnergyMeter::readEnergyFromMsr() const
{
    uint64_t value;
    int ret =  readMsr(getMsrInterface(), msr_package_energy_offset, &value);
    if (ret < 0) {
        qWarning() << "Read msr rapl pwoer unit failed.";
        return 0;
    }

    return (value & 0xffffffff) * readRaplPowerUnit();
}

double CpuRaplEnergyMeter::readRaplPowerUnit() const
{
    uint64_t value;
    int ret = readMsr(getMsrInterface(), msr_power_unit_offset, &value);
    if (ret < 0) {
        qWarning() << "Read msr rapl power unit failed.";
        return 0;
    }

    return (double)1/ std::pow((double)2, (double)((value & 0x1f00) >> 8));
}

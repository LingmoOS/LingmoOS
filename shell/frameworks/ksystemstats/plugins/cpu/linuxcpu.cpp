/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "linuxcpu.h"

#include <QFile>

#include <sensors/sensors.h>
#include <systemstats/SensorsFeatureSensor.h>

static double readCpuFreq(const QString &cpuId, const QString &attribute, bool &ok)
{
    ok = false;
    QFile file(QStringLiteral("/sys/devices/system/cpu/%1/cpufreq/").arg(cpuId)  + attribute);
    bool open = file.open(QIODevice::ReadOnly);
    if (open) {
        // Remove trailing new line
        return file.readAll().chopped(1).toUInt(&ok) / 1000.0; // CPUFreq reports values in kHZ
    }
    return 0;
}

LinuxCpuObject::LinuxCpuObject(const QString &id, const QString &name, double initialFrequency, KSysGuard::SensorContainer *parent)
    : CpuObject(id, name, parent)
    , m_initialFrequency(initialFrequency)
{
}

void LinuxCpuObject::makeTemperatureSensor(const sensors_chip_name * const chipName, const sensors_feature * const feature)
{
    m_temperature = KSysGuard::makeSensorsFeatureSensor(QStringLiteral("temperature"), chipName, feature, this);
}

void LinuxCpuObject::initialize()
{
    CpuObject::initialize();
    m_frequency->setValue(m_initialFrequency);
    bool ok;
    const double max = readCpuFreq(id(), QStringLiteral("cpuinfo_max_freq"), ok);
    if (ok) {
        m_frequency->setMax(max);
    }
    const double min = readCpuFreq(id(), QStringLiteral("cpuinfo_min_freq"), ok);
    if (ok) {
        m_frequency->setMin(min);
    }
}

void LinuxCpuObject::makeSensors()
{
    BaseCpuObject::makeSensors();
    m_frequency = new KSysGuard::SensorProperty(QStringLiteral("frequency"), this);
    if (!m_temperature) {
        m_temperature = new KSysGuard::SensorProperty(QStringLiteral("temperature"), this);
    }
}

void LinuxCpuObject::update(long long system, long long user, long long wait, long long idle)
{
    if (!isSubscribed()) {
        return;
    }

    // First update usages
    m_usageComputer.setTicks(system, user, wait, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_wait->setValue(m_usageComputer.waitUsage);
    m_usage->setValue(m_usageComputer.totalUsage);

    // Second try to get current frequency
    bool ok = false;
    // First try cpuinfo_cur_freq, it is the frequency the hardware runs at (https://www.kernel.org/doc/html/latest/admin-guide/pm/cpufreq.html)
    int frequency = readCpuFreq(id(), QStringLiteral("cpuinfo_cur_freq"), ok);
    if (!ok) {
        frequency = readCpuFreq(id(), QStringLiteral("scaling_cur_freq"), ok);
    }
    if (ok) {
        m_frequency->setValue(frequency);
    }
    // FIXME Should we fall back to reading /proc/cpuinfo again when the above fails? Could have the 
    // frequency value changed even if the cpu apparently doesn't use CPUFreq?

    // Third update temperature
    m_temperature->update();
}

void LinuxAllCpusObject::update(long long system, long long user, long long wait, long long idle) {
    m_usageComputer.setTicks(system, user, wait, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_wait->setValue(m_usageComputer.waitUsage);
    m_usage->setValue(m_usageComputer.totalUsage);
}

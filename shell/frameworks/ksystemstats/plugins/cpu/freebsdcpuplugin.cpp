/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "freebsdcpuplugin.h"

#include "loadaverages.h"

#include <algorithm>
#include <vector>

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/sysctl.h>

#include <KLocalizedString>

#include <systemstats/SensorContainer.h>
#include <systemstats/SysctlSensor.h>

namespace {

/** Reads a sysctl into a typed buffer, return success-value
 *
 * Returns @c false if the sysctl fails, otherwise @c true (even if
 * the returned size is a mismatch or whatever).
 */
template <typename T>
bool readSysctl(const char *name, T *buffer, size_t size = sizeof(T)) {
    return sysctlbyname(name, buffer, &size, nullptr, 0) != -1;
}

/** Calls update() with sysctl cp_time data
 *
 * For a CPU object, or an AllCpus object, calls update() with the relevant data.
 */
template<typename cpu_t>
inline void updateCpu(cpu_t *cpu, long *cp_time)
{
    cpu->update(cp_time[CP_SYS] + cp_time[CP_INTR], cp_time[CP_USER] + cp_time[CP_NICE], cp_time[CP_IDLE]);
}

/** Reads cp_times from sysctl and applies to the vector of CPU objects
 *
 * Assumes that the CPU objects are ordered in the vector in the same order
 * that their data show up in the sysctl return value.
 */
inline void read_cp_times(QList<FreeBsdCpuObject *> &cpus)
{
    unsigned int numCores = cpus.count();
    std::vector<long> cp_times(numCores * CPUSTATES);
    size_t cpTimesSize = sizeof(long) *  cp_times.size();
    if (readSysctl("kern.cp_times", cp_times.data(), cpTimesSize)) {//, &cpTimesSize, nullptr, 0) != -1) {
        for (unsigned int  i = 0; i < numCores; ++i) {
            auto cpu = cpus[i];
            updateCpu(cpu, &cp_times[CPUSTATES * i]);
        }
    }
}

}

FreeBsdCpuObject::FreeBsdCpuObject(int cpuNumber, const QString &name, KSysGuard::SensorContainer *parent)
    : CpuObject(QStringLiteral("cpu%1").arg(cpuNumber), name, parent),
    m_sysctlPrefix(QByteArrayLiteral("dev.cpu.") + QByteArray::number(cpuNumber))
{
}

void FreeBsdCpuObject::makeSensors()
{
    BaseCpuObject::makeSensors();

    m_frequency = new KSysGuard::SysctlSensor<int>(QStringLiteral("frequency"), m_sysctlPrefix + QByteArrayLiteral(".freq"), 0, this);
    m_temperature = new KSysGuard::SysctlSensor<int>(QStringLiteral("temperature"), m_sysctlPrefix + QByteArrayLiteral(".temperature"), 0, this);
}

void FreeBsdCpuObject::initialize()
{
    CpuObject::initialize();

    // For min and max frequency we have to parse the values return by freq_levels because only
    // minimum is exposed as a single value
    size_t size;
    const QByteArray levelsName = m_sysctlPrefix + QByteArrayLiteral(".freq_levels");
    // calling sysctl with nullptr writes the needed size to size
    if (sysctlbyname(levelsName, nullptr, &size, nullptr, 0) != -1) {
        QByteArray freqLevels(size, Qt::Uninitialized);
        if (sysctlbyname(levelsName, freqLevels.data(), &size, nullptr, 0) != -1) {
            // The format is a list of pairs "frequency/power", see https://svnweb.freebsd.org/base/head/sys/kern/kern_cpu.c?revision=360464&view=markup#l1019
            const QList<QByteArray> levels = freqLevels.split(' ');
            int min = INT_MAX;
            int max = 0;
            for (const auto &level : levels) {
                const int frequency = level.left(level.indexOf('/')).toInt();
                min = std::min(frequency, min);
                max = std::max(frequency, max);
            }
            // value are already in MHz  see cpufreq(4)
            m_frequency->setMin(min);
            m_frequency->setMax(max);
        }
    }
    const QByteArray tjmax = m_sysctlPrefix + QByteArrayLiteral(".coretemp.tjmax");
    int maxTemperature;
    // This is only availabel on Intel (using the coretemp driver)
    if (readSysctl(tjmax.constData(), &maxTemperature)) {
        m_temperature->setMax(maxTemperature);
    }
}

void FreeBsdCpuObject::update(long system, long user, long idle)
{
    // No wait usage on FreeBSD
    m_usageComputer.setTicks(system, user, 0, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_usage->setValue(m_usageComputer.totalUsage);

    // The calculations above are "free" because we already have the data;
    // is we are not subscribed, don't bother updating the data that needs
    // extra work to be done.
    if (!isSubscribed()) {
        return;
    }

    m_temperature->update();
    m_frequency->update();
}

void FreeBsdAllCpusObject::update(long system, long user, long idle)
{
    // No wait usage on FreeBSD
    m_usageComputer.setTicks(system, user, 0, idle);

    m_system->setValue(m_usageComputer.systemUsage);
    m_user->setValue(m_usageComputer.userUsage);
    m_usage->setValue(m_usageComputer.totalUsage);
}

FreeBsdCpuPluginPrivate::FreeBsdCpuPluginPrivate(CpuPlugin* q)
    : CpuPluginPrivate(q)
{
    m_loadAverages = new LoadAverages(m_container);
    // The values used here can be found in smp(4)
    int numCpu;
    readSysctl("hw.ncpu", &numCpu);
    for (int i = 0; i < numCpu; ++i) {
        auto cpu = new FreeBsdCpuObject(i, i18nc("@title", "CPU %1", i + 1), m_container);
        cpu->initialize();
        m_cpus.push_back(cpu);
    }
    m_allCpus = new FreeBsdAllCpusObject(m_container);
    m_allCpus->initialize();

    read_cp_times(m_cpus);
}

void FreeBsdCpuPluginPrivate::update()
{
    m_loadAverages->update();

    auto isSubscribed = [] (const KSysGuard::SensorObject *o) {return o->isSubscribed();};
    if (std::none_of(m_cpus.cbegin(), m_cpus.cend(), isSubscribed) && !m_allCpus->isSubscribed()) {
        return;
    }
    read_cp_times(m_cpus);
    // update total values
    long cp_time[CPUSTATES];
    if (readSysctl("kern.cp_time", &cp_time)) {
        updateCpu(m_allCpus, cp_time);
    }
}

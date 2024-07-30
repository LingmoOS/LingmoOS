/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "linuxcpuplugin.h"

#include <QFile>

#include <KLocalizedString>

#include <systemstats/SensorContainer.h>

#include <sensors/sensors.h>

#include "linuxcpu.h"
#include "loadaverages.h"

struct CpuInfo
{
    int id = -1;
    int cpu = -1;
    int core = -1;
    int siblings = -1;
    qreal frequency = 0.0;
};

// Determine sensor names for all the found processors. Because processors can
// be offline, we need to account for processor IDs skipping and report the
// proper names.
static QHash<int, QString> makeCpuNames(const QList<CpuInfo> &cpus, int cpuCount)
{
    QHash<int, QString> result;

    if (cpuCount == 1) {
        // Simple case: Only one CPU, just report CPU number + 1 as core number.
        for (const auto &info : cpus) {
            result.insert(info.id, i18nc("@title", "Core %1", info.id + 1));
        }
        return result;
    }

    int currentCpu = 0;
    int previousCpuSiblings = 0;

    for (const auto &info : cpus) {
        if (info.cpu != currentCpu) {
            previousCpuSiblings = previousCpuSiblings + info.siblings;
            currentCpu = info.cpu;
        }

        int coreNumber = info.id - previousCpuSiblings;
        result.insert(info.id, i18nc("@title", "CPU %1 Core %2", currentCpu + 1, coreNumber));
    }

    return result;
}

LinuxCpuPluginPrivate::LinuxCpuPluginPrivate(CpuPlugin *q)
    : CpuPluginPrivate(q)
{
    m_loadAverages = new LoadAverages(m_container);

    // Parse /proc/cpuinfo for information about cpus
    QFile cpuinfo(QStringLiteral("/proc/cpuinfo"));
    cpuinfo.open(QIODevice::ReadOnly);

    int cpuCount = 0;
    QList<CpuInfo> cpus;

    for (QByteArray line = cpuinfo.readLine(); !line.isEmpty(); line = cpuinfo.readLine()) {
        CpuInfo info;
        // Processors are divided by empty lines
        for (; line != "\n";  line = cpuinfo.readLine()) {
            // we are interested in processor number as identifier for /proc/stat, physical id (the
            // cpu the core belongs to) and the number of the core. However with hyperthreading
            // multiple entries will have the same combination of physical id and core id. So we just
            // count up the core number. For mapping temperature both ids are still needed nonetheless.
            const int delim = line.indexOf(":");
            const QByteArray field = line.left(delim).trimmed();
            const QByteArray value = line.mid(delim + 1).trimmed();
            if (field == "processor") {
                info.id = value.toInt();
            } else if (field == "physical id") {
                info.cpu = value.toInt();
            } else if (field == "core id") {
                info.core = value.toInt();
            } else if (field == "cpu MHz") {
                info.frequency = value.toDouble();
            } else if (field == "siblings") {
                info.siblings = value.toInt();
            }
        }

        cpus.push_back(info);
        cpuCount = std::max(cpuCount, info.cpu);
    }

    // cpuCount is based on the indices of the cpus, which means it is off by
    // one compared to the actual number of CPUs. Correct that here.
    cpuCount += 1;

    auto names = makeCpuNames(cpus, cpuCount);

    QHash<int, int> numCores;
    for (const auto &entry : std::as_const(cpus)) {
        auto cpu = new LinuxCpuObject(QStringLiteral("cpu%1").arg(entry.id), names.value(entry.id), entry.frequency, m_container);
        m_cpus.insert(entry.id, cpu);
        m_cpusBySystemIds.insert({entry.cpu, entry.core}, cpu);
    }

    addSensors();
    for (const auto cpu : std::as_const(m_cpus)) {
        cpu->initialize();
    }
    m_allCpus = new LinuxAllCpusObject(m_container);
    m_allCpus->initialize();
    m_allCpus->setCounts(cpuCount, m_cpus.size());

}

void LinuxCpuPluginPrivate::update()
{
    m_loadAverages->update();

    auto isSubscribed = [] (const KSysGuard::SensorObject *o) {return o->isSubscribed();};
    if (std::none_of(m_cpus.cbegin(), m_cpus.cend(), isSubscribed) && !m_allCpus->isSubscribed()) {
        return;
    }

    // Parse /proc/stat to get usage values. The format is described at
    // https://www.kernel.org/doc/html/latest/filesystems/proc.html#miscellaneous-kernel-statistics-in-proc-stat
    QFile stat(QStringLiteral("/proc/stat"));
    stat.open(QIODevice::ReadOnly);
    for (QByteArray line = stat.readLine(); !line.isNull(); line = stat.readLine()) {
        auto values = line.simplified().split(' ');
        if (!line.startsWith("cpu")) {
            continue;
        }

        long long user = values[1].toLongLong();
        long long nice = values[2].toLongLong();
        long long system = values[3].toLongLong();
        long long idle = values[4].toLongLong();
        long long iowait = values[5].toLongLong();
        long long irq = values[6].toLongLong();
        long long softirq = values[7].toLongLong();
        long long steal = values[8].toLongLong();

        // Total values just start with "cpu", single cpus are numbered cpu0, cpu1, ...
        if (line.startsWith("cpu ")) {
            m_allCpus->update(system + irq + softirq, user + nice , iowait + steal, idle);
        } else if (line.startsWith("cpu")) {
            auto cpu = m_cpus.value(std::atoi(line.mid(strlen("cpu"))));
            cpu->update(system + irq + softirq, user + nice , iowait + steal, idle);
        }
    }
}


void LinuxCpuPluginPrivate::addSensors()
{
    int number = 0;
    while (const sensors_chip_name * const chipName = sensors_get_detected_chips(nullptr, &number)) {
        char name[100];
        sensors_snprintf_chip_name(name, 100, chipName);
        if (qstrcmp(chipName->prefix, "coretemp") == 0) {
            addSensorsIntel(chipName);
        } else if (qstrcmp(chipName->prefix, "k10temp") == 0) {
            addSensorsAmd(chipName);
        }
    }
}

// Documentation: https://www.kernel.org/doc/html/latest/hwmon/coretemp.html
void LinuxCpuPluginPrivate::addSensorsIntel(const sensors_chip_name * const chipName)
{
    int featureNumber = 0;
    QHash<std::pair<unsigned int, unsigned int>,  sensors_feature const *> coreFeatures;
    int physicalId = 0;
    while (sensors_feature const * feature = sensors_get_features(chipName, &featureNumber)) {
        if (feature->type != SENSORS_FEATURE_TEMP) {
            continue;
        }
        char * sensorLabel = sensors_get_label(chipName, feature);
        unsigned int coreId;
        // First try to see if it's a core temperature because we should have more of those
        if (std::sscanf(sensorLabel, "Core %d", &coreId) != 0) {
            coreFeatures.insert(std::make_pair(physicalId, coreId), feature);
        } else {
            std::sscanf(sensorLabel, "Package id %d", &physicalId);
        }
        free(sensorLabel);
    }

    for (auto feature = coreFeatures.cbegin(); feature != coreFeatures.cend(); ++feature) {
        if (m_cpusBySystemIds.contains(feature.key())) {
            // When the cpu has hyperthreading we display multiple cores for each physical core.
            // Naturally they share the same temperature sensor and have the same coreId.
            auto cpu_range = m_cpusBySystemIds.equal_range(feature.key());
            for (auto cpu_it = cpu_range.first; cpu_it != cpu_range.second; ++cpu_it) {
                (*cpu_it)->makeTemperatureSensor(chipName, feature.value());
            }
        }
    }
}

void LinuxCpuPluginPrivate::addSensorsAmd(const sensors_chip_name * const chipName)
{
    // All Processors should have the Tctl pseudo temperature as temp1. Newer ones have the real die
    // temperature Tdie as temp2. Some of those have temperatures for each core complex die (CCD) as
    // temp3-6 or temp3-10 depending on the number of CCDS.
    // https://www.kernel.org/doc/html/latest/hwmon/k10temp.html
    int featureNumber = 0;
    sensors_feature const * tctl = nullptr;
    sensors_feature const * tdie = nullptr;
    sensors_feature const * tccd[8] = {nullptr};
    while (sensors_feature const * feature = sensors_get_features(chipName, &featureNumber)) {
        const QByteArray name (feature->name);
        if (feature->type != SENSORS_FEATURE_TEMP || !name.startsWith("temp")) {
            continue;
        }
        // For temps 1 and 2 we can't just go by the number because in  kernels older than 5.7 they
        // are the wrong way around, so we have to compare labels.
        // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=b02c6857389da66b09e447103bdb247ccd182456
        char * label = sensors_get_label(chipName, feature);
        if (qstrcmp(label, "Tctl") == 0 || qstrcmp(label, "temp1") == 0) {
            tctl = feature;
        } else if (qstrcmp(label, "Tdie") == 0 || qstrcmp(label, "temp2") == 0) {
            tdie = feature;
        } else if (qstrncmp(label, "Tccd", 4) == 0) {
            tccd[name.mid(4).toUInt()] = feature;
        } else {
            qWarning() << "Unrecognised temmperature sensor: " << label;
        }
        free(label);
    }
    // TODO How to map CCD temperatures to cores?

    auto setSingleSensor = [this, chipName] (const sensors_feature * const feature) {
        for (auto &cpu : std::as_const(m_cpusBySystemIds)) {
            cpu->makeTemperatureSensor(chipName, feature);
        }
    };
    if (tdie) {
        setSingleSensor(tdie);
    } else if (tctl) {
        setSingleSensor(tctl);
    }
}

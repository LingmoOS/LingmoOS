/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FREEBSDCPU_H
#define FREEBSDCPU_H

#include "cpu.h"
#include "cpuplugin_p.h"
#include "usagecomputer.h"

namespace KSysGuard {
template <typename T>
class SysctlSensor;
}
class LoadAverages;

class FreeBsdCpuObject : public CpuObject {
public:
    FreeBsdCpuObject(int cpuNumber, const QString &name, KSysGuard::SensorContainer *parent);
    void update(long system, long user, long idle);
    void initialize() override;
private:
    void makeSensors() override;

    UsageComputer m_usageComputer;
    const QByteArray m_sysctlPrefix;  // "dev.cpu.<n>"
};

class FreeBsdAllCpusObject : public AllCpusObject {
public:
    using AllCpusObject::AllCpusObject;
    void update(long system, long user, long idle);
private:
    UsageComputer m_usageComputer;
};

class FreeBsdCpuPluginPrivate : public CpuPluginPrivate {
public:
    FreeBsdCpuPluginPrivate(CpuPlugin *q);
    void update() override;
private:
    FreeBsdAllCpusObject *m_allCpus;
    QList<FreeBsdCpuObject *> m_cpus;
    LoadAverages *m_loadAverages;
};

#endif

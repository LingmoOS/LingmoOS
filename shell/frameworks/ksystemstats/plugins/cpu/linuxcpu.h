/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LINUXCPU_H
#define LINUXCPU_H

#include "cpu.h"
#include "usagecomputer.h"


struct sensors_chip_name;
struct sensors_feature;
class SensorsFeatureSensor;

class LinuxCpuObject : public CpuObject
{
public:
    LinuxCpuObject(const QString &id, const QString &name, double initialFrequency, KSysGuard::SensorContainer *parent);

    void update(long long system, long long user, long long wait, long long idle);
    void initialize() override;
    void makeTemperatureSensor(const sensors_chip_name * constchipName, const sensors_feature * const feature);
private:
    void makeSensors() override;

    double m_initialFrequency;
    UsageComputer m_usageComputer;
};

class LinuxAllCpusObject : public AllCpusObject {
public:
    using AllCpusObject::AllCpusObject;
    void update(long long system, long long user, long long wait, long long idle);
private:
    UsageComputer m_usageComputer;
};

#endif

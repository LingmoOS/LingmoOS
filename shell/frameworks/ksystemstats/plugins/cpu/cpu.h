/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CPU_H
#define CPU_H

#include <systemstats/SensorObject.h>

class BaseCpuObject : public KSysGuard::SensorObject {
public:
protected:
    BaseCpuObject(const QString &id, const QString &name, KSysGuard::SensorContainer *parent);

    virtual void initialize();
    virtual void makeSensors();

    KSysGuard::SensorProperty *m_usage = nullptr;
    KSysGuard::SensorProperty *m_system = nullptr;
    KSysGuard::SensorProperty *m_user = nullptr;
    KSysGuard::SensorProperty *m_wait = nullptr;
};

class CpuObject : public BaseCpuObject {
public:
    CpuObject(const QString &id, const QString &name, KSysGuard::SensorContainer *parent);
    void initialize() override;
protected:
    void makeSensors() override;

    KSysGuard::SensorProperty *m_frequency = nullptr;
    KSysGuard::SensorProperty *m_temperature = nullptr;
};

class AllCpusObject : public BaseCpuObject {
public:
    AllCpusObject(KSysGuard::SensorContainer *parent);
    void setCounts(unsigned int cpuCount, unsigned int coreCount);
    void initialize() override;
protected:
    void makeSensors() override;

    KSysGuard::SensorProperty *m_cpuCount = nullptr;
    KSysGuard::SensorProperty *m_coreCount = nullptr;
};

#endif

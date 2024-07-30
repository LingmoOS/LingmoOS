/*
    SPDX-FileCopyrightText: 2023 Adrian Edwards <adrian@adriancedwards.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <systemstats/SensorPlugin.h>

#include <systemstats/SensorContainer.h>
#include <systemstats/SensorObject.h>
#include <systemstats/SensorProperty.h>

class PressurePlugin : public KSysGuard::SensorPlugin
{
    Q_OBJECT
public:
    PressurePlugin(QObject *parent, const QVariantList &args);
    ~PressurePlugin() override;

    QString providerName() const override
    {
        return QStringLiteral("pressure");
    }

    void update() override;

private:
    PressurePlugin *q;

    KSysGuard::SensorContainer *container = nullptr;

    KSysGuard::SensorObject *memoryObject = nullptr;
    KSysGuard::SensorProperty *memorySome10SecProperty = nullptr;
    KSysGuard::SensorProperty *memorySome60SecProperty = nullptr;
    KSysGuard::SensorProperty *memorySome300SecProperty = nullptr;
    KSysGuard::SensorProperty *memorySomeTotalProperty = nullptr;

    KSysGuard::SensorProperty *memoryFull10SecProperty = nullptr;
    KSysGuard::SensorProperty *memoryFull60SecProperty = nullptr;
    KSysGuard::SensorProperty *memoryFull300SecProperty = nullptr;
    KSysGuard::SensorProperty *memoryFullTotalProperty = nullptr;


    KSysGuard::SensorObject *cpuObject = nullptr;
    KSysGuard::SensorProperty *cpuSome10SecProperty = nullptr;
    KSysGuard::SensorProperty *cpuSome60SecProperty = nullptr;
    KSysGuard::SensorProperty *cpuSome300SecProperty = nullptr;
    KSysGuard::SensorProperty *cpuSomeTotalProperty = nullptr;

    KSysGuard::SensorProperty *cpuFull10SecProperty = nullptr;
    KSysGuard::SensorProperty *cpuFull60SecProperty = nullptr;
    KSysGuard::SensorProperty *cpuFull300SecProperty = nullptr;
    KSysGuard::SensorProperty *cpuFullTotalProperty = nullptr;


    KSysGuard::SensorObject *ioObject = nullptr;
    KSysGuard::SensorProperty *ioSome10SecProperty = nullptr;
    KSysGuard::SensorProperty *ioSome60SecProperty = nullptr;
    KSysGuard::SensorProperty *ioSome300SecProperty = nullptr;
    KSysGuard::SensorProperty *ioSomeTotalProperty = nullptr;

    KSysGuard::SensorProperty *ioFull10SecProperty = nullptr;
    KSysGuard::SensorProperty *ioFull60SecProperty = nullptr;
    KSysGuard::SensorProperty *ioFull300SecProperty = nullptr;
    KSysGuard::SensorProperty *ioFullTotalProperty = nullptr;
};

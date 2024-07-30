/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "systemstats/SensorObject.h"

namespace KSysGuard
{
    class AggregateSensor;
}

class AllGpus : public KSysGuard::SensorObject
{
    Q_OBJECT

public:
    AllGpus(KSysGuard::SensorContainer *parent);

private:
    KSysGuard::AggregateSensor *m_usageSensor = nullptr;
    KSysGuard::AggregateSensor *m_totalVramSensor = nullptr;
    KSysGuard::AggregateSensor *m_usedVramSensor = nullptr;
};

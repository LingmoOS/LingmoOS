/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <systemstats/SensorObject.h>

class NetworkDevice;

namespace KSysGuard
{
    class AggregateSensor;
}

/**
 * This object aggregates the network usage of all devices.
 */
class AllDevicesObject : public KSysGuard::SensorObject
{
    Q_OBJECT

public:
    AllDevicesObject(KSysGuard::SensorContainer* parent);

private:
    KSysGuard::AggregateSensor *m_downloadSensor = nullptr;
    KSysGuard::AggregateSensor *m_uploadSensor = nullptr;
    KSysGuard::AggregateSensor *m_downloadBitsSensor = nullptr;
    KSysGuard::AggregateSensor *m_uploadBitsSensor = nullptr;
    KSysGuard::AggregateSensor *m_totalDownloadSensor = nullptr;
    KSysGuard::AggregateSensor *m_totalUploadSensor = nullptr;
};

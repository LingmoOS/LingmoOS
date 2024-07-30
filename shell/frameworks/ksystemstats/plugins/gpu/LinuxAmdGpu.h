/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "GpuDevice.h"

struct udev_device;

namespace KSysGuard
{
    class SysFsSensor;
}

class LinuxAmdGpu : public GpuDevice
{
    Q_OBJECT

public:
    LinuxAmdGpu(const QString& id, const QString& name, udev_device *device);
    ~LinuxAmdGpu() override;

    void initialize() override;
    void update() override;

protected:
    void makeSensors() override;

private:
    void discoverSensors();

    udev_device *m_device;
    QList<KSysGuard::SysFsSensor *> m_sysFsSensors;
    QList<KSysGuard::SensorProperty *> m_sensorsSensors;
};

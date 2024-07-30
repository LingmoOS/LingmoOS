/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "GpuDevice.h"
#include "NvidiaSmiProcess.h"

struct udev_device;

class LinuxNvidiaGpu : public GpuDevice
{
    Q_OBJECT

public:
    LinuxNvidiaGpu(const QString& id, const QString &name, udev_device *device);
    ~LinuxNvidiaGpu() override;

    void initialize() override;

private:
    void onDataReceived(const NvidiaSmiProcess::GpuData &data);

    int m_index = -1;
    udev_device *m_device;

    static NvidiaSmiProcess *s_smiProcess;
};

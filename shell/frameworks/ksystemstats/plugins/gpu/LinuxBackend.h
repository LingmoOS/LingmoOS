/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "GpuBackend.h"

struct udev;
class GpuDevice;

class LinuxBackend : public GpuBackend
{
    Q_OBJECT

public:
    LinuxBackend(QObject* parent = nullptr);

    void start() override;
    void stop() override;
    void update() override;

    int deviceCount() override;

private:
    udev *m_udev = nullptr;
    QList<GpuDevice *> m_devices;
};

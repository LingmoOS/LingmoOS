/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

class GpuDevice;

class GpuBackend : public QObject
{
    Q_OBJECT

public:
    GpuBackend(QObject* parent = nullptr) : QObject(parent) { }

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void update() = 0;

    virtual int deviceCount() = 0;

    Q_SIGNAL void deviceAdded(GpuDevice *device);
    Q_SIGNAL void deviceRemoved(GpuDevice *device);
};

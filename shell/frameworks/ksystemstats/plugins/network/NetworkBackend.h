/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

class NetworkDevice;

class NetworkBackend : public QObject
{
    Q_OBJECT

public:
    NetworkBackend(QObject* parent = nullptr) : QObject(parent) { }
    ~NetworkBackend() override = default;

    virtual bool isSupported() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void update() {};

    Q_SIGNAL void deviceAdded(NetworkDevice *device);
    Q_SIGNAL void deviceRemoved(NetworkDevice *device);
};

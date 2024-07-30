/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <memory>
#include <QHash>

#include "NetworkBackend.h"
#include "NetworkDevice.h"

class QTimer;

namespace NetworkManager
{
    class Device;
    class DeviceStatistics;
    class WirelessDevice;
}

class NetworkManagerDevice : public NetworkDevice
{
    Q_OBJECT

public:
    NetworkManagerDevice(const QString &id, QSharedPointer<NetworkManager::Device> device);
    ~NetworkManagerDevice() override;

    void update();

    bool isConnected() const;

    Q_SIGNAL void connected(NetworkManagerDevice *device);
    Q_SIGNAL void disconnected(NetworkManagerDevice *device);

private:
    void updateWifi();

    QSharedPointer<NetworkManager::Device> m_device;
    QSharedPointer<NetworkManager::DeviceStatistics> m_statistics;
    NetworkManager::WirelessDevice *m_wifiDevice = nullptr;
    std::unique_ptr<QTimer> m_statisticsTimer;
    bool m_connected = false;
    bool m_restoreTimer = false;
    uint m_initialStatisticsRate;
};

class NetworkManagerBackend : public NetworkBackend
{
    Q_OBJECT

public:
    NetworkManagerBackend(QObject *parent = nullptr);
    ~NetworkManagerBackend() override;

    bool isSupported() override;
    void start() override;
    void stop() override;

private:
    void onDeviceAdded(const QString &uni);
    void onDeviceRemoved(const QString &uni);

    QHash<QString, NetworkManagerDevice *> m_devices;
};


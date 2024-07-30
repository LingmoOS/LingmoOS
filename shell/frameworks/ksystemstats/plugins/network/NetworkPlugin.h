/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "systemstats/SensorPlugin.h"

class NetworkPrivate;
class NetworkDevice;

class NetworkPlugin : public KSysGuard::SensorPlugin
{
    Q_OBJECT
public:
    NetworkPlugin(QObject *parent, const QVariantList &args);
    ~NetworkPlugin() override;

    QString providerName() const override
    {
        return QStringLiteral("network");
    }

    void onDeviceAdded(NetworkDevice *device);
    void onDeviceRemoved(NetworkDevice *device);

    void update() override;

private:
    std::unique_ptr<NetworkPrivate> d;
};

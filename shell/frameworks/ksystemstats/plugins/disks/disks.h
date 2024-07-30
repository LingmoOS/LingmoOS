/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DISKS_H
#define DISKS_H

#include <QObject>
#include <QElapsedTimer>

#include "systemstats/SensorPlugin.h"

namespace Solid {
    class Device;
    class StorageVolume;
}

class VolumeObject;

class DisksPlugin : public KSysGuard::SensorPlugin

{
    Q_OBJECT
public:
    DisksPlugin(QObject *parent, const QVariantList &args);
    QString providerName() const override
    {
        return QStringLiteral("disks");
    }
    ~DisksPlugin() override;

    void update() override;


private:
    void addDevice(const Solid::Device &device);
    void addAggregateSensors();
    void createAccessibleVolumeObject(const Solid::Device &device);

    QHash<QString, VolumeObject*> m_volumesByDevice;
    QElapsedTimer m_elapsedTimer;
};

#endif

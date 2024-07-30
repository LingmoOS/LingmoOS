/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <memory>

#include <systemstats/SensorPlugin.h>

class GpuPlugin : public KSysGuard::SensorPlugin
{
    Q_OBJECT
public:
    GpuPlugin(QObject *parent, const QVariantList &args);
    ~GpuPlugin();

    QString providerName() const override
    {
        return QStringLiteral("gpu");
    }

    void update() override;

private:
    class Private;
    std::unique_ptr<Private> d;
};

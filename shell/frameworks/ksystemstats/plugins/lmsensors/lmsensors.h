/*
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef LMSENSORS_H
#define LMSENSORS_H

#include <systemstats/SensorPlugin.h>

namespace KSysGuard
{
class SensorsFeatureSensor;
}
class LmSensorsPlugin : public KSysGuard::SensorPlugin
{
    Q_OBJECT
public:
    LmSensorsPlugin(QObject *parent, const QVariantList &args);
    ~LmSensorsPlugin() override;
    QString providerName() const override;
    void update() override;
private:
    QList<KSysGuard::SensorsFeatureSensor *> m_sensors;
};
#endif

/*
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef SENSORSFEATURESENSOR_H
#define SENSORSFEATURESENSOR_H

#include "SensorProperty.h"
#include "systemstats_export.h"

#ifdef Q_OS_LINUX

struct sensors_chip_name;
struct sensors_feature;
struct sensors_subfeature;

namespace KSysGuard
{
/**
 * Convenience subclass of SensorProperty that wraps a sensor from a libsensors
 * @since 5.23
 */
class SYSTEMSTATS_EXPORT SensorsFeatureSensor : public SensorProperty
{
    Q_OBJECT
public:
    /**
     * Update the sensor by reading the current value.
     */
    void update() override;
    ~SensorsFeatureSensor() override = default;

private:
    SensorsFeatureSensor(const QString &id, const sensors_chip_name *const chipName, const sensors_subfeature *const valueFeature, SensorObject *parent);
    const sensors_chip_name *m_chipName;
    const sensors_subfeature *m_valueFeature;
    friend SensorsFeatureSensor *makeSensorsFeatureSensor(const QString &id, const sensors_chip_name *const, const sensors_feature *const, SensorObject *);
};

/**
 * Creates a SensorsFeatureSensor wrapping the sensor represented by @p chipName and @p feature with
 * the given @p id and @p parent as parent.
 * This function will set the unit of the returned SensorProperty and determines the minimum and maximum if
 * available. If the label of @p feature is of the form 'temp%d', 'in%d' or 'fan%d' the name will be
 * 'Temperature %d', 'Voltage %d' or 'Fan %d' respectively, otherwise the label will be used as name.
 * @returns A pointer to a new SensorsFeatureSensor or @c nullptr if @p parent already contains a sensor
 * with @p id, the sensor does not support reading the current value or @c feature->type is greater or
 * equal @c SENSORS_FEATURE_MAX_MAIN.
 * @since 5.23
 */
SYSTEMSTATS_EXPORT SensorsFeatureSensor *
makeSensorsFeatureSensor(const QString &id, const sensors_chip_name *const chipName, const sensors_feature *const feature, SensorObject *parent);
}
#endif
#endif

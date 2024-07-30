/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <memory>

#include <QObject>

#include "SensorProperty.h"
#include "systemstats_export.h"

namespace KSysGuard
{
/**
 * Convenience subclass of SensorProperty that reads a sysfs file and uses the result as value.
 */
class SYSTEMSTATS_EXPORT SysFsSensor : public SensorProperty
{
    Q_OBJECT

public:
    SysFsSensor(const QString &id, const QString &path, SensorObject *parent);
    SysFsSensor(const QString &id, const QString &path, const QVariant &initialValue, SensorObject *parent);
    ~SysFsSensor() override;

    /**
     * Set the function used to convert the data from sysfs to the value of this sensor.
     *
     * This accepts a function that takes a QByteArray and converts that to a QVariant.
     * By default this is set to `std::atoll` or in other words, any numeric value
     * should automatically be converted to a proper QVariant.
     */
    void setConvertFunction(const std::function<QVariant(const QByteArray &)> &function);

    /**
     * Update this sensor.
     *
     * This will cause the sensor to read sysfs and update the value from that.
     * It should be called periodically so values are updated properly.
     */
    void update() override;

private:
    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard

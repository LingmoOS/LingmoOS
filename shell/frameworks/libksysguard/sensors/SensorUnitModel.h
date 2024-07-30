/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <memory>

#include <QAbstractListModel>
#include <qqmlregistration.h>

#include "sensors_export.h"

namespace KSysGuard
{
class SensorInfo;

/**
 * A model that lists the units for a given list of sensors.
 *
 * This model will provide a list of units for a given list of sensors. It
 * includes all the prefixes starting at the base unit of a sensor. For example,
 * for a sensor with unit "bytes" it will list "bytes", "kilobytes", "megabytes"
 * etc. If instead the unit were "megabytes", the model would list "megabytes",
 * "gigabytes", etc. The units for all sensors are listed, with duplicates
 * removed. They are sorted based on the order of units in Formatter's Unit.h.
 *
 * Three roles are exposed:
 * - "unit": The actual Unit enum value for this unit.
 * - "symbol": The textual symbol for the unit, like "KiB/s".
 * - "multiplier": The multiplier to apply to the unit value to convert it to
 *                 the base unit. For example, to convert from kilobytes to
 *                 bytes the multiplier would be 1024.
 */
class SENSORS_EXPORT SensorUnitModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    /**
     * The list of sensors to list units for.
     */
    Q_PROPERTY(QStringList sensors READ sensors WRITE setSensors NOTIFY sensorsChanged)
    /**
     * Indicates the model has retrieved all the information of the requested sensors.
     *
     * This will be false right after setting the value of `sensors`. It will
     * change to true once the information for all sensors has been retrieved.
     */
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

public:
    enum Roles {
        UnitRole = Qt::UserRole,
        SymbolRole,
        MultiplierRole,
    };
    Q_ENUM(Roles)

    SensorUnitModel(QObject *parent = nullptr);
    ~SensorUnitModel() override;

    QStringList sensors() const;
    void setSensors(const QStringList &newSensors);
    Q_SIGNAL void sensorsChanged();

    bool ready() const;
    Q_SIGNAL void readyChanged();

    QHash<int, QByteArray> roleNames() const override;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

private:
    void metaDataChanged(const QString &id, const SensorInfo &info);

    class Private;
    const std::unique_ptr<Private> d;
};

}

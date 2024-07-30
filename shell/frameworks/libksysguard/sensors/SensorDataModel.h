/*
    SPDX-FileCopyrightText: 2019 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "sensors_export.h"
#include <QAbstractTableModel>
#include <QDateTime>
#include <QQmlParserStatus>
#include <memory>
#include <qqmlregistration.h>

namespace KSysGuard
{
class SensorInfo;

/**
 * A model representing a table of sensors.
 *
 * This model will expose the metadata and values of a list of sensors as a
 * table, using one column for each sensor. The metadata and values are
 * represented as different roles.
 */
class SENSORS_EXPORT SensorDataModel : public QAbstractTableModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

    /**
     * The list of sensors to watch.
     */
    Q_PROPERTY(QStringList sensors READ sensors WRITE setSensors NOTIFY sensorsChanged)
    /**
     * The minimum value of all sensors' minimum property.
     */
    Q_PROPERTY(qreal minimum READ minimum NOTIFY sensorMetaDataChanged)
    /**
     * The maximum value of all sensors' maximum property.
     */
    Q_PROPERTY(qreal maximum READ maximum NOTIFY sensorMetaDataChanged)
    /**
     * Should this model be updated or not. Defaults to true.
     */
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    /**
     * Is this model ready?
     *
     * Ready means it has retrieved the metadata for the sensors specified in
     * \ref sensors.
     */
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

    /**
     * Used by the model to provide data for the Color role if set.
     */
    Q_PROPERTY(QVariantMap sensorColors READ sensorColors WRITE setSensorColors NOTIFY sensorColorsChanged)

    /**
     * Provides custom labels for Sensors that are used instead of the name and short name of the sensors.
     */
    Q_PROPERTY(QVariantMap sensorLabels READ sensorLabels WRITE setSensorLabels NOTIFY sensorLabelsChanged)

    /**
     * The minimum time between updates, in milliseconds.
     *
     * If this is set to a positive non-zero value, at least this many
     * milliseconds need to elapse before another value update happens, otherwise
     * it is ignored. This effectively rate-limits the updates and will prevent
     * value updates.
     */
    Q_PROPERTY(int updateRateLimit READ updateRateLimit WRITE setUpdateRateLimit NOTIFY updateRateLimitChanged RESET resetUpdateRateLimit)

public:
    /**
     * The roles that this model exposes
     * @see Sensor
     */
    enum AdditionalRoles {
        SensorId = Qt::UserRole + 1, ///< The backend path to the sensor.
        Name, ///< The name of the sensor.
        ShortName, ///< A shorter name for the sensor. This is equal to name if not set.
        Description, ///< A description for the sensor.
        Unit, ///< The unit of the sensor.
        Minimum, ///< The minimum value this sensor can have.
        Maximum, ///< The maximum value this sensor can have.
        Type, ///< The QVariant::Type of the sensor.
        Value, ///< The value of the sensor.
        FormattedValue, ///< A formatted string of the value of the sensor.
        Color, ///< A color of the sensor, if sensorColors is set
        UpdateInterval, ///< The time in milliseconds between each update of the sensor.
    };
    Q_ENUM(AdditionalRoles)

    explicit SensorDataModel(const QStringList &sensorIds = {}, QObject *parent = nullptr);
    ~SensorDataModel() override;

    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QStringList sensors() const;
    void setSensors(const QStringList &sensorIds);
    Q_SIGNAL void sensorsChanged() const;
    Q_SIGNAL void sensorMetaDataChanged();

    bool enabled() const;
    void setEnabled(bool newEnabled);
    Q_SIGNAL void enabledChanged();

    qreal minimum() const;
    qreal maximum() const;

    QVariantMap sensorColors() const;
    void setSensorColors(const QVariantMap &sensorColors);
    Q_SIGNAL void sensorColorsChanged();

    QVariantMap sensorLabels() const;
    void setSensorLabels(const QVariantMap &sensorLabels);
    Q_SIGNAL void sensorLabelsChanged();

    int updateRateLimit() const;
    void setUpdateRateLimit(int newUpdateRateLimit);
    void resetUpdateRateLimit();
    Q_SIGNAL void updateRateLimitChanged();

    bool isReady() const;
    Q_SIGNAL void readyChanged();

    Q_INVOKABLE void addSensor(const QString &sensorId);
    Q_INVOKABLE void removeSensor(const QString &sensorId);
    Q_INVOKABLE int column(const QString &sensorId) const;

    void classBegin() override;
    void componentComplete() override;

private:
    void onSensorAdded(const QString &sensorId);
    void onSensorRemoved(const QString &sensorId);
    void onMetaDataChanged(const QString &sensorId, const SensorInfo &info);
    void onValueChanged(const QString &sensorId, const QVariant &value);

    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard

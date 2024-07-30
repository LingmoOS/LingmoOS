/*
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "SensorsFeatureSensor.h"

#include "SensorObject.h"

#include <KLocalizedString>

#include <QRegularExpression>

#include <sensors/sensors.h>

static QString prettyName(const sensors_chip_name *const chipName, const sensors_feature *const feature)
{
    std::unique_ptr<char, decltype(&free)> label(sensors_get_label(chipName, feature), &free);
    const QString labelString = QString::fromUtf8(label.get());
    switch (feature->type) {
    case SENSORS_FEATURE_IN: {
        static QRegularExpression inRegex(QStringLiteral("in(\\d+)"));
        auto inMatch = inRegex.match(labelString);
        if (inMatch.hasMatch()) {
            return i18nc("@title %1 is a number", "Voltage %1", inMatch.captured(1));
        }
        break;
    }
    case SENSORS_FEATURE_FAN: {
        static QRegularExpression fanRegex(QStringLiteral("fan(\\d+)"));
        auto fanMatch = fanRegex.match(labelString);
        if (fanMatch.hasMatch()) {
            return i18nc("@title %1 is a number", "Fan %1", fanMatch.captured(1));
        }
        break;
    }
    case SENSORS_FEATURE_TEMP: {
        static QRegularExpression tempRegex(QStringLiteral("temp(\\d+)"));
        auto tempMatch = tempRegex.match(labelString);
        if (tempMatch.hasMatch()) {
            return i18nc("@title %1 is a number", "Temperature %1", tempMatch.captured(1));
        }
        break;
    }
    default:
        break;
    }
    return labelString;
}

namespace KSysGuard
{
SensorsFeatureSensor *
makeSensorsFeatureSensor(const QString &id, const sensors_chip_name *const chipName, const sensors_feature *const feature, SensorObject *parent)
{
    if (parent->sensor(id)) {
        return nullptr;
    }

    const sensors_subfeature *valueFeature = nullptr;
    double minimum = 0;
    double maximum = 0;
    Unit unit;

    auto getValueOfFirstExisting = [chipName, feature](std::initializer_list<sensors_subfeature_type> subfeatureTypes) {
        double value;
        for (auto subfeatureType : subfeatureTypes) {
            const sensors_subfeature *const subfeature = sensors_get_subfeature(chipName, feature, subfeatureType);
            if (subfeature && sensors_get_value(chipName, subfeature->number, &value) == 0) {
                return value;
            }
        }
        return 0.0;
    };

    switch (feature->type) {
    case SENSORS_FEATURE_IN:
        valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_IN_INPUT);
        unit = UnitVolt;
        minimum = getValueOfFirstExisting({SENSORS_SUBFEATURE_IN_LCRIT, SENSORS_SUBFEATURE_IN_MIN});
        maximum = getValueOfFirstExisting({SENSORS_SUBFEATURE_IN_CRIT, SENSORS_SUBFEATURE_IN_MAX});
        break;
    case SENSORS_FEATURE_FAN:
        valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_FAN_INPUT);
        unit = UnitRpm;
        minimum = getValueOfFirstExisting({SENSORS_SUBFEATURE_FAN_MIN});
        maximum = getValueOfFirstExisting({SENSORS_SUBFEATURE_FAN_MAX});
        break;
    case SENSORS_FEATURE_TEMP:
        valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_TEMP_INPUT);
        unit = UnitCelsius;
        minimum = getValueOfFirstExisting({SENSORS_SUBFEATURE_TEMP_LCRIT, SENSORS_SUBFEATURE_TEMP_MIN});
        maximum = getValueOfFirstExisting({SENSORS_SUBFEATURE_TEMP_EMERGENCY, SENSORS_SUBFEATURE_TEMP_CRIT, SENSORS_SUBFEATURE_TEMP_MAX});
        break;
    case SENSORS_FEATURE_POWER:
        valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_POWER_INPUT);
        if (!valueFeature) {
            valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_POWER_AVERAGE);
        }
        unit = UnitWatt;
        maximum = getValueOfFirstExisting({SENSORS_SUBFEATURE_POWER_CRIT, SENSORS_SUBFEATURE_POWER_MAX});
        break;
    case SENSORS_FEATURE_ENERGY:
        valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_ENERGY_INPUT);
        unit = UnitWattHour;
        break;
    case SENSORS_FEATURE_CURR:
        valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_CURR_INPUT);
        unit = UnitAmpere;
        minimum = getValueOfFirstExisting({SENSORS_SUBFEATURE_CURR_LCRIT, SENSORS_SUBFEATURE_CURR_MIN});
        maximum = getValueOfFirstExisting({SENSORS_SUBFEATURE_CURR_CRIT, SENSORS_SUBFEATURE_CURR_MAX});
        break;
    case SENSORS_FEATURE_HUMIDITY:
        valueFeature = sensors_get_subfeature(chipName, feature, SENSORS_SUBFEATURE_HUMIDITY_INPUT);
        unit = UnitPercent;
        break;
    default:
        return nullptr;
    }

    if (!valueFeature) {
        return nullptr;
    }

    auto sensor = new SensorsFeatureSensor(id, chipName, valueFeature, parent);
    sensor->setName(prettyName(chipName, feature));
    sensor->setUnit(unit);
    sensor->setMax(maximum);
    sensor->setMin(minimum);

    return sensor;
}

SensorsFeatureSensor::SensorsFeatureSensor(const QString &id,
                                           const sensors_chip_name *const chipName,
                                           const sensors_subfeature *const valueFeature,
                                           SensorObject *parent)
    : SensorProperty(id, id, 0, parent)
    , m_chipName(chipName)
    , m_valueFeature(valueFeature)
{
}

void SensorsFeatureSensor::update()
{
    if (!isSubscribed()) {
        return;
    }

    double value;
    if (sensors_get_value(m_chipName, m_valueFeature->number, &value) < 0) {
        setValue(QVariant());
    }
    setValue(value);
}
}

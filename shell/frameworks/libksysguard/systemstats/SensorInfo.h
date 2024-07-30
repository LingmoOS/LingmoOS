/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDBusArgument>
#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QVariant>

#include "formatter/Unit.h"

#include "systemstats_export.h"

namespace KSysGuard
{
constexpr uint BackendUpdateInterval = 500;

// Data that is static for the lifespan of the sensor
class SYSTEMSTATS_EXPORT SensorInfo
{
public:
    SensorInfo() = default;

    QString name; // Translated name of the sensor.
    QString shortName; // Shorter translated name of the sensor, to use when space is constrained.
    QString description; // Translated description of the sensor.
    QVariant::Type variantType = QVariant::Invalid;
    KSysGuard::Unit unit = KSysGuard::UnitInvalid; // Both a format hint and implies data type (i.e double/string)
    qreal min = 0;
    qreal max = 0;
};

class SYSTEMSTATS_EXPORT SensorData
{
public:
    SensorData() = default;
    SensorData(const QString &_sensorProperty, const QVariant &_payload)
        : sensorProperty(_sensorProperty)
        , payload(_payload)
    {
    }
    QString sensorProperty;
    QVariant payload;
};

typedef QHash<QString, SensorInfo> SensorInfoMap;
typedef QList<SensorData> SensorDataList;

inline QDBusArgument &operator<<(QDBusArgument &argument, const SensorInfo &s)
{
    argument.beginStructure();
    argument << s.name;
    argument << s.shortName;
    argument << s.description;
    argument << s.variantType;
    argument << s.unit;
    argument << s.min;
    argument << s.max;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SensorInfo &s)
{
    argument.beginStructure();
    argument >> s.name;
    argument >> s.shortName;
    argument >> s.description;
    uint32_t t;
    argument >> t;
    s.variantType = static_cast<QVariant::Type>(t);
    argument >> t;
    s.unit = static_cast<KSysGuard::Unit>(t);
    argument >> s.min;
    argument >> s.max;
    argument.endStructure();
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const SensorData &s)
{
    argument.beginStructure();
    argument << s.sensorProperty;
    argument << QDBusVariant(s.payload);
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SensorData &s)
{
    argument.beginStructure();
    argument >> s.sensorProperty;
    argument >> s.payload;
    argument.endStructure();
    return argument;
}

} // namespace KSysGuard

Q_DECLARE_METATYPE(KSysGuard::SensorInfo);
Q_DECLARE_METATYPE(KSysGuard::SensorData);
Q_DECLARE_METATYPE(KSysGuard::SensorDataList);

/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

// Qt
#include <QMetaType>
#include <qqmlregistration.h>

#include "formatter_export.h"

namespace KSysGuard
{
FORMATTER_EXPORT Q_NAMESPACE QML_NAMED_ELEMENT(Units)

    /**
     * This enum type is used to specify metric prefixes.
     */
    enum MetricPrefix {
        MetricPrefixAutoAdjust = -1,
        MetricPrefixUnity = 0,
        MetricPrefixKilo,
        MetricPrefixMega,
        MetricPrefixGiga,
        MetricPrefixTera,
        MetricPrefixPeta,
        MetricPrefixLast = MetricPrefixPeta
    };
Q_ENUM_NS(MetricPrefix)

/**
 * This enum types is used to specify units.
 */
enum Unit {
    UnitInvalid = -1,
    UnitNone = 0,

    // Byte size units.
    UnitByte = 100,
    UnitKiloByte = MetricPrefixKilo + UnitByte,
    UnitMegaByte = MetricPrefixMega + UnitByte,
    UnitGigaByte = MetricPrefixGiga + UnitByte,
    UnitTeraByte = MetricPrefixTera + UnitByte,
    UnitPetaByte = MetricPrefixPeta + UnitByte,

    // Data rate units.
    UnitByteRate = 200,
    UnitKiloByteRate = MetricPrefixKilo + UnitByteRate,
    UnitMegaByteRate = MetricPrefixMega + UnitByteRate,
    UnitGigaByteRate = MetricPrefixGiga + UnitByteRate,
    UnitTeraByteRate = MetricPrefixTera + UnitByteRate,
    UnitPetaByteRate = MetricPrefixPeta + UnitByteRate,

    // Frequency.
    UnitHertz = 300,
    UnitKiloHertz = MetricPrefixKilo + UnitHertz,
    UnitMegaHertz = MetricPrefixMega + UnitHertz,
    UnitGigaHertz = MetricPrefixGiga + UnitHertz,
    UnitTeraHertz = MetricPrefixTera + UnitHertz,
    UnitPetaHertz = MetricPrefixPeta + UnitHertz,

    // Time units.
    UnitBootTimestamp = 400,
    UnitSecond,
    UnitTime,
    UnitTicks,
    UnitDuration,

    // Data rate units in bits.
    UnitBitRate = 500,
    UnitKiloBitRate = MetricPrefixKilo + UnitBitRate,
    UnitMegaBitRate = MetricPrefixMega + UnitBitRate,
    UnitGigaBitRate = MetricPrefixGiga + UnitBitRate,
    UnitTeraBitRate = MetricPrefixTera + UnitBitRate,
    UnitPetaBitRate = MetricPrefixPeta + UnitBitRate,

    // Volt.
    UnitVolt = 600,
    UnitKiloVolt = MetricPrefixKilo + UnitVolt,
    UnitMegaVolt = MetricPrefixMega + UnitVolt,
    UnitGigaVolt = MetricPrefixGiga + UnitVolt,
    UnitTeraVolt = MetricPrefixTera + UnitVolt,
    UnitPetaVolt = MetricPrefixPeta + UnitVolt,

    // Watt.
    UnitWatt = 700,
    UnitKiloWatt = MetricPrefixKilo + UnitWatt,
    UnitMegaWatt = MetricPrefixMega + UnitWatt,
    UnitGigaWatt = MetricPrefixGiga + UnitWatt,
    UnitTeraWatt = MetricPrefixTera + UnitWatt,
    UnitPetaWatt = MetricPrefixPeta + UnitWatt,

    // WattHour.
    UnitWattHour = 800,
    UnitKiloWattHour = MetricPrefixKilo + UnitWattHour,
    UnitMegaWattHour = MetricPrefixMega + UnitWattHour,
    UnitGigaWattHour = MetricPrefixGiga + UnitWattHour,
    UnitTeraWattHour = MetricPrefixTera + UnitWattHour,
    UnitPetaWattHour = MetricPrefixPeta + UnitWattHour,

    // Ampere.
    UnitAmpere = 900,
    UnitKiloAmpere = MetricPrefixKilo + UnitAmpere,
    UnitMegaAmpere = MetricPrefixMega + UnitAmpere,
    UnitGigaAmpere = MetricPrefixGiga + UnitAmpere,
    UnitTeraAmpere = MetricPrefixTera + UnitAmpere,
    UnitPetaAmpere = MetricPrefixPeta + UnitAmpere,

    // Misc units.
    UnitCelsius = 1000,
    UnitDecibelMilliWatts,
    UnitPercent,
    UnitRate,
    UnitRpm,
};
Q_ENUM_NS(Unit)

} // namespace KSysGuard

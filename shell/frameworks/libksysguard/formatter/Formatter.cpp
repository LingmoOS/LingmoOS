/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    formatBootTimestamp is based on TimeUtil class:
    SPDX-FileCopyrightText: 2014 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "Formatter.h"

#include <KFormat>
#include <KLocalizedString>

#include <QFontMetrics>
#include <QLocale>
#include <QTime>

#include <cmath>
#include <ctime>

#include <time.h>
#include <unistd.h>

#include "formatter_debug.h"

namespace KSysGuard
{
// TODO: Is there a bit nicer way to handle formatting?

static KLocalizedString unitFormat(Unit unit)
{
    // The strings here use \u200B (zero-width space) to allow the unit symbol
    // to be wrapped to a new line when there is little space and potentially
    // even be hidden that way. Since the prefix is part of the unit it risks
    // hiding the prefix, so the zero-width space is placed after the prefix and
    // we use \u202F (narrow no-break space) to ensure we have some separation
    // but do not wrap it. For the prefix-less units where we *do* want to allow
    // wrapping, we use \u2009 (thin space) to ensure we have the same spacing
    // between prefix-less and prefixed units.

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString B = ki18nc("Bytes unit symbol", "%1\u2009B");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString KiB = ki18nc("Kilobytes unit symbol", "%1\u202FKi\u200BB");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString MiB = ki18nc("Megabytes unit symbol", "%1\u202FMi\u200BB");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString GiB = ki18nc("Gigabytes unit symbol", "%1\u202FGi\u200BB");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString TiB = ki18nc("Terabytes unit symbol", "%1\u202FTi\u200BB");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString PiB = ki18nc("Petabytes unit symbol", "%1\u202FPi\u200BB");

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString bps = ki18nc("Bytes per second unit symbol", "%1\u2009B/s");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Kbps = ki18nc("Kilobytes per second unit symbol", "%1\u202FKi\u200BB/s");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Mbps = ki18nc("Megabytes per second unit symbol", "%1\u202FMi\u200BB/s");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Gbps = ki18nc("Gigabytes per second unit symbol", "%1\u202FGi\u200BB/s");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Tbps = ki18nc("Terabytes per second unit symbol", "%1\u202FTi\u200BB/s");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Pbps = ki18nc("Petabytes per second unit symbol", "%1\u202FPi\u200BB/s");

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString bitsps = ki18nc("Bits per second unit symbol", "%1\u2009bps");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Kbitsps = ki18nc("Kilobits per second unit symbol", "%1\u202Fk\u200Bbps");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Mbitsps = ki18nc("Megabits per second unit symbol", "%1\u202FM\u200Bbps");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Gbitsps = ki18nc("Gigabits per second unit symbol", "%1\u202FG\u200Bbps");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Tbitsps = ki18nc("Terabits per second unit symbol", "%1\u202FT\u200Bbps");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString Pbitsps = ki18nc("Petabits per second unit symbol", "%1\u202FP\u200Bbps");

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString Hz = ki18nc("Hertz unit symbol", "%1\u2009Hz");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString kHz = ki18nc("Kilohertz unit symbol", "%1\u202Fk\u200BHz");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString MHz = ki18nc("Megahertz unit symbol", "%1\u202FM\u200BHz");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString GHz = ki18nc("Gigahertz unit symbol", "%1\u202FG\u200BHz");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString THz = ki18nc("Terahertz unit symbol", "%1\u202FT\u200BHz");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString PHz = ki18nc("Petahertz unit symbol", "%1\u202FP\u200BHz");

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString V = ki18nc("Volts unit symbol", "%1\u2009V");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString kV = ki18nc("Kilovolts unit symbol", "%1\u202Fk\u200BV");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString MV = ki18nc("Megavolts unit symbol", "%1\u202FM\u200BV");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString GV = ki18nc("Gigavolts unit symbol", "%1\u202FG\u200BV");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString TV = ki18nc("Teravolts unit symbol", "%1\u202FT\u200BV");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString PV = ki18nc("Petavolts unit symbol", "%1\u202FP\u200BV");

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString W = ki18nc("Watts unit symbol", "%1\u2009W");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString kW = ki18nc("Kilowatts unit symbol", "%1\u202Fk\u200BW");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString MW = ki18nc("Megawatts unit symbol", "%1\u202FM\u200BW");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString GW = ki18nc("Gigawatts unit symbol", "%1\u202FG\u200BW");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString TW = ki18nc("Terawatts unit symbol", "%1\u202FT\u200BW");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString PW = ki18nc("Petawatts unit symbol", "%1\u202FP\u200BW");

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString Wh = ki18nc("Watt-hours unit symbol", "%1\u2009Wh");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString kWh = ki18nc("Kilowatt-hours unit symbol", "%1\u202Fk\u200BWh");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString MWh = ki18nc("Megawatt-hours unit symbol", "%1\u202FM\u200BWh");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString GWh = ki18nc("Gigawatt-hours unit symbol", "%1\u202FG\u200BWh");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString TWh = ki18nc("Terawatt-hours unit symbol", "%1\u202FT\u200BWh");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString PWh = ki18nc("Petawatt-hours unit symbol", "%1\u202FP\u200BWh");

    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString A = ki18nc("Ampere unit symbol", "%1\u2009A");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString kA = ki18nc("Kiloamperes unit symbol", "%1\u202Fk\u200BA");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString MA = ki18nc("Megaamperes unit symbol", "%1\u202FM\u200BA");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString GA = ki18nc("Gigaamperes unit symbol", "%1\u202FG\u200BA");
    // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString TA = ki18nc("Teraamperes unit symbol", "%1\u202FT\u200BA");
    // // i18n: Use \u202F (thin non-break space) to separate value and prefixed unit; Use \u200B (zero-width space) to separate prefix and unit.
    const static KLocalizedString PA = ki18nc("Petaamperes unit symbol", "%1\u202FP\u200BA");

    // i18n: Use \u200B (zero width space) to separate value and unit
    const static KLocalizedString percent = ki18nc("Percent unit", "%1\u200B%");
    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString RPM = ki18nc("Revolutions per minute unit symbol", "%1\u2009RPM");
    // i18n: Use \u200B (zero width space) to separate value and unit
    const static KLocalizedString C = ki18nc("Celsius unit symbol", "%1\u200B°C");
    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString dBm = ki18nc("Decibels unit symbol", "%1\u2009dBm");
    // i18n: Use \u200B (zero width space) to separate value and unit
    const static KLocalizedString s = ki18nc("Seconds unit symbol", "%1\u200Bs");
    // i18n: Use \u2009 (thin space) to separate value and unit
    const static KLocalizedString rate = ki18nc("Rate unit symbol", "%1\u2009s⁻¹");
    const static KLocalizedString unitless = ki18nc("Unitless", "%1");

    switch (unit) {
    case UnitByte:
        return B;
    case UnitKiloByte:
        return KiB;
    case UnitMegaByte:
        return MiB;
    case UnitGigaByte:
        return GiB;
    case UnitTeraByte:
        return TiB;
    case UnitPetaByte:
        return PiB;

    case UnitByteRate:
        return bps;
    case UnitKiloByteRate:
        return Kbps;
    case UnitMegaByteRate:
        return Mbps;
    case UnitGigaByteRate:
        return Gbps;
    case UnitTeraByteRate:
        return Tbps;
    case UnitPetaByteRate:
        return Pbps;

    case UnitBitRate:
        return bitsps;
    case UnitKiloBitRate:
        return Kbitsps;
    case UnitMegaBitRate:
        return Mbitsps;
    case UnitGigaBitRate:
        return Gbitsps;
    case UnitTeraBitRate:
        return Tbitsps;
    case UnitPetaBitRate:
        return Pbitsps;

    case UnitHertz:
        return Hz;
    case UnitKiloHertz:
        return kHz;
    case UnitMegaHertz:
        return MHz;
    case UnitGigaHertz:
        return GHz;
    case UnitTeraHertz:
        return THz;
    case UnitPetaHertz:
        return PHz;

    case UnitVolt:
        return V;
    case UnitKiloVolt:
        return kV;
    case UnitMegaVolt:
        return MV;
    case UnitGigaVolt:
        return GV;
    case UnitTeraVolt:
        return TV;
    case UnitPetaVolt:
        return PV;

    case UnitWatt:
        return W;
    case UnitKiloWatt:
        return kW;
    case UnitMegaWatt:
        return MW;
    case UnitGigaWatt:
        return GW;
    case UnitTeraWatt:
        return TW;
    case UnitPetaWatt:
        return PV;

    case UnitWattHour:
        return Wh;
    case UnitKiloWattHour:
        return kWh;
    case UnitMegaWattHour:
        return MWh;
    case UnitGigaWattHour:
        return GWh;
    case UnitTeraWattHour:
        return TWh;
    case UnitPetaWattHour:
        return PWh;

    case UnitAmpere:
        return A;
    case UnitKiloAmpere:
        return kA;
    case UnitMegaAmpere:
        return MA;
    case UnitGigaAmpere:
        return GA;
    case UnitTeraAmpere:
        return TA;
    case UnitPetaAmpere:
        return PA;

    case UnitCelsius:
        return C;
    case UnitDecibelMilliWatts:
        return dBm;
    case UnitPercent:
        return percent;
    case UnitRate:
        return rate;
    case UnitRpm:
        return RPM;
    case UnitSecond:
        return s;

    default:
        return unitless;
    }
}

static int unitOrder(Unit unit)
{
    switch (unit) {
    case UnitByte:
    case UnitKiloByte:
    case UnitMegaByte:
    case UnitGigaByte:
    case UnitTeraByte:
    case UnitPetaByte:
    case UnitByteRate:
    case UnitKiloByteRate:
    case UnitMegaByteRate:
    case UnitGigaByteRate:
    case UnitTeraByteRate:
    case UnitPetaByteRate:
    case UnitBitRate:
    case UnitKiloBitRate:
    case UnitMegaBitRate:
    case UnitGigaBitRate:
    case UnitTeraBitRate:
    case UnitPetaBitRate:
        return 1024;

    case UnitHertz:
    case UnitKiloHertz:
    case UnitMegaHertz:
    case UnitGigaHertz:
    case UnitTeraHertz:
    case UnitPetaHertz:

    case UnitWatt:
    case UnitKiloWatt:
    case UnitMegaWatt:
    case UnitGigaWatt:
    case UnitTeraWatt:
    case UnitPetaWatt:

    case UnitWattHour:
    case UnitKiloWattHour:
    case UnitMegaWattHour:
    case UnitGigaWattHour:
    case UnitTeraWattHour:
    case UnitPetaWattHour:

    case UnitAmpere:
    case UnitKiloAmpere:
    case UnitMegaAmpere:
    case UnitGigaAmpere:
    case UnitTeraAmpere:
    case UnitPetaAmpere:

    case UnitVolt:
    case UnitKiloVolt:
    case UnitMegaVolt:
    case UnitGigaVolt:
    case UnitTeraVolt:
    case UnitPetaVolt:
        return 1000;

    default:
        return 0;
    }
}

static Unit unitBase(Unit unit)
{
    switch (unit) {
    case UnitByte:
    case UnitKiloByte:
    case UnitMegaByte:
    case UnitGigaByte:
    case UnitTeraByte:
    case UnitPetaByte:
        return UnitByte;

    case UnitByteRate:
    case UnitKiloByteRate:
    case UnitMegaByteRate:
    case UnitGigaByteRate:
    case UnitTeraByteRate:
    case UnitPetaByteRate:
        return UnitByteRate;

    case UnitBitRate:
    case UnitKiloBitRate:
    case UnitMegaBitRate:
    case UnitGigaBitRate:
    case UnitTeraBitRate:
    case UnitPetaBitRate:
        return UnitBitRate;

    case UnitHertz:
    case UnitKiloHertz:
    case UnitMegaHertz:
    case UnitGigaHertz:
    case UnitTeraHertz:
    case UnitPetaHertz:
        return UnitHertz;

    case UnitVolt:
    case UnitKiloVolt:
    case UnitMegaVolt:
    case UnitGigaVolt:
    case UnitTeraVolt:
    case UnitPetaVolt:
        return UnitVolt;

    case UnitWatt:
    case UnitKiloWatt:
    case UnitMegaWatt:
    case UnitGigaWatt:
    case UnitTeraWatt:
    case UnitPetaWatt:
        return UnitWatt;

    case UnitWattHour:
    case UnitKiloWattHour:
    case UnitMegaWattHour:
    case UnitGigaWattHour:
    case UnitTeraWattHour:
    case UnitPetaWattHour:
        return UnitWattHour;

    case UnitAmpere:
    case UnitKiloAmpere:
    case UnitMegaAmpere:
    case UnitGigaAmpere:
    case UnitTeraAmpere:
    case UnitPetaAmpere:
        return UnitAmpere;

    default:
        return unit;
    }
}

static Unit adjustedUnit(qreal value, Unit unit, MetricPrefix prefix)
{
    const int order = unitOrder(unit);
    if (!order) {
        return unit;
    }

    const Unit baseUnit = unitBase(unit);
    const MetricPrefix basePrefix = MetricPrefix(unit - baseUnit);

    if (prefix == MetricPrefixAutoAdjust) {
        const qreal absoluteValue = value * std::pow(order, int(basePrefix));
        if (absoluteValue > 0) {
            const int targetPrefix = std::log2(absoluteValue) / std::log2(order);
            if (targetPrefix <= MetricPrefixLast) {
                prefix = MetricPrefix(targetPrefix);
            }
        }
        if (prefix == MetricPrefixAutoAdjust) {
            prefix = basePrefix;
        }
    }

    const Unit newUnit = Unit(int(prefix) + int(baseUnit));
    // If there is no prefixed unit,
    // don't overflow into the following unrelated units.
    if (unitBase(newUnit) != baseUnit) {
        return unit;
    }

    return newUnit;
}

static QString formatNumber(const QVariant &value, Unit unit, MetricPrefix prefix, FormatOptions options, int precision)
{
    qreal amount = value.toDouble();

    if (!options.testFlag(FormatOptionShowNull) && (qFuzzyIsNull(amount) || qIsNaN(amount))) {
        return QString();
    }

    const Unit adjusted = adjustedUnit(amount, unit, prefix);
    if (adjusted != unit) {
        amount /= std::pow(unitOrder(unit), adjusted - unit);
    }

    if (precision < 0) {
        precision = (value.typeId() != QMetaType::Double && adjusted <= unit) ? 0 : 1;
    }
    const QString text = QLocale().toString(amount, 'f', precision);

    return unitFormat(adjusted).subs(text).toString();
}

static QString formatTime(const QVariant &value)
{
    return KFormat().formatDuration(value.toLongLong() * 1000);
}

static QString formatTicks(const QVariant &value)
{
    auto seconds = value.toLongLong() / sysconf(_SC_CLK_TCK);
    return KFormat().formatDuration(seconds * 1000);
}

static QString formatBootTimestamp(const QVariant &value)
{
    timespec tp;
#ifdef Q_OS_LINUX
    clock_gettime(CLOCK_BOOTTIME, &tp);
#else
    clock_gettime(CLOCK_MONOTONIC, &tp);
#endif

    const QDateTime systemBootTime = QDateTime::currentDateTime().addSecs(-tp.tv_sec);

    const qreal secondsSinceSystemBoot = value.toReal() / sysconf(_SC_CLK_TCK);
    const QDateTime absoluteTimeSinceBoot = systemBootTime.addSecs(secondsSinceSystemBoot);

    return KFormat().formatRelativeDateTime(absoluteTimeSinceBoot, QLocale::ShortFormat);
}

qreal Formatter::scaleDownFactor(const QVariant &value, Unit unit, MetricPrefix targetPrefix)
{
    const Unit adjusted = adjustedUnit(value.toDouble(), unit, targetPrefix);
    if (adjusted == unit) {
        return 1;
    }

    return std::pow(unitOrder(unit), adjusted - unit);
}

KLocalizedString Formatter::localizedString(const QVariant &value, Unit unit, MetricPrefix targetPrefix)
{
    const Unit adjusted = adjustedUnit(value.toDouble(), unit, targetPrefix);
    return unitFormat(adjusted);
}

QString Formatter::formatValue(const QVariant &value, Unit unit, MetricPrefix targetPrefix, FormatOptions options, int precision)
{
    switch (unit) {
    case UnitByte:
    case UnitKiloByte:
    case UnitMegaByte:
    case UnitGigaByte:
    case UnitTeraByte:
    case UnitPetaByte:
    case UnitByteRate:
    case UnitKiloByteRate:
    case UnitMegaByteRate:
    case UnitGigaByteRate:
    case UnitTeraByteRate:
    case UnitPetaByteRate:
    case UnitBitRate:
    case UnitKiloBitRate:
    case UnitMegaBitRate:
    case UnitGigaBitRate:
    case UnitTeraBitRate:
    case UnitPetaBitRate:
    case UnitHertz:
    case UnitKiloHertz:
    case UnitMegaHertz:
    case UnitGigaHertz:
    case UnitTeraHertz:
    case UnitPetaHertz:
    case UnitVolt:
    case UnitKiloVolt:
    case UnitMegaVolt:
    case UnitGigaVolt:
    case UnitTeraVolt:
    case UnitPetaVolt:
    case UnitWatt:
    case UnitKiloWatt:
    case UnitMegaWatt:
    case UnitGigaWatt:
    case UnitTeraWatt:
    case UnitPetaWatt:
    case UnitWattHour:
    case UnitKiloWattHour:
    case UnitMegaWattHour:
    case UnitGigaWattHour:
    case UnitTeraWattHour:
    case UnitPetaWattHour:
    case UnitAmpere:
    case UnitKiloAmpere:
    case UnitMegaAmpere:
    case UnitGigaAmpere:
    case UnitTeraAmpere:
    case UnitPetaAmpere:
    case UnitSecond:
    case UnitPercent:
    case UnitRate:
    case UnitRpm:
    case UnitCelsius:
    case UnitDecibelMilliWatts:
        return formatNumber(value, unit, targetPrefix, options, precision);

    case UnitBootTimestamp:
        return formatBootTimestamp(value);
    case UnitTime:
        return formatTime(value);
    case UnitNone:
        return formatNumber(value, unit, MetricPrefix::MetricPrefixUnity, options, precision);
    case UnitTicks:
        return formatTicks(value);

    default:
        return value.toString();
    }
}

QString Formatter::symbol(Unit unit)
{
    // TODO: Is it possible to avoid duplication of these symbols?
    switch (unit) {
    case UnitByte:
        return i18nc("Bytes unit symbol", "B");
    case UnitKiloByte:
        return i18nc("Kilobytes unit symbol", "KiB");
    case UnitMegaByte:
        return i18nc("Megabytes unit symbol", "MiB");
    case UnitGigaByte:
        return i18nc("Gigabytes unit symbol", "GiB");
    case UnitTeraByte:
        return i18nc("Terabytes unit symbol", "TiB");
    case UnitPetaByte:
        return i18nc("Petabytes unit symbol", "PiB");

    case UnitByteRate:
        return i18nc("Bytes per second unit symbol", "B/s");
    case UnitKiloByteRate:
        return i18nc("Kilobytes per second unit symbol", "KiB/s");
    case UnitMegaByteRate:
        return i18nc("Megabytes per second unit symbol", "MiB/s");
    case UnitGigaByteRate:
        return i18nc("Gigabytes per second unit symbol", "GiB/s");
    case UnitTeraByteRate:
        return i18nc("Terabytes per second unit symbol", "TiB/s");
    case UnitPetaByteRate:
        return i18nc("Petabytes per second unit symbol", "PiB/s");

    case UnitBitRate:
        return i18nc("Bits per second unit symbol", "bps");
    case UnitKiloBitRate:
        return i18nc("Kilobits per second unit symbol", "Kbps");
    case UnitMegaBitRate:
        return i18nc("Megabits per second unit symbol", "Mbps");
    case UnitGigaBitRate:
        return i18nc("Gigabits per second unit symbol", "Gbps");
    case UnitTeraBitRate:
        return i18nc("Terabits per second unit symbol", "Tbps");
    case UnitPetaBitRate:
        return i18nc("Petabits per second unit symbol", "Pbps");

    case UnitHertz:
        return i18nc("Hertz unit symbol", "Hz");
    case UnitKiloHertz:
        return i18nc("Kilohertz unit symbol", "kHz");
    case UnitMegaHertz:
        return i18nc("Megahertz unit symbol", "MHz");
    case UnitGigaHertz:
        return i18nc("Gigahertz unit symbol", "GHz");
    case UnitTeraHertz:
        return i18nc("Terahertz unit symbol", "THz");
    case UnitPetaHertz:
        return i18nc("Petahertz unit symbol", "PHz");

    case UnitVolt:
        return i18nc("Volts unit symbol", "V");
    case UnitKiloVolt:
        return i18nc("Kilovolts unit symbol", "kV");
    case UnitMegaVolt:
        return i18nc("Megavolts unit symbol", "MV");
    case UnitGigaVolt:
        return i18nc("Gigavolts unit symbol", "GV");
    case UnitTeraVolt:
        return i18nc("Teravolts unit symbol", "TV");
    case UnitPetaVolt:
        return i18nc("Petavolts unit symbol", "PV");

    case UnitWatt:
        return i18nc("Watts unit symbol", "W");
    case UnitKiloWatt:
        return i18nc("Kilowatts unit symbol", "kW");
    case UnitMegaWatt:
        return i18nc("Megawatts unit symbol", "MW");
    case UnitGigaWatt:
        return i18nc("Gigawatts unit symbol", "GW");
    case UnitTeraWatt:
        return i18nc("Terawatts unit symbol", "TW");
    case UnitPetaWatt:
        return i18nc("Petawatts unit symbol", "PW");

    case UnitWattHour:
        return i18nc("Watt-hours unit symbol", "Wh");
    case UnitKiloWattHour:
        return i18nc("Kilo-watthours unit symbol", "kWh");
    case UnitMegaWattHour:
        return i18nc("Mega-watthours unit symbol", "MWh");
    case UnitGigaWattHour:
        return i18nc("Giga-watthours unit symbol", "GWh");
    case UnitTeraWattHour:
        return i18nc("Tera-watthours unit symbol", "TWh");
    case UnitPetaWattHour:
        return i18nc("Peta-watthours unit symbol", "PWh");

    case UnitAmpere:
        return i18nc("Ampere unit symbol", "A");
    case UnitKiloAmpere:
        return i18nc("Kiloamperes unit symbol", "kA");
    case UnitMegaAmpere:
        return i18nc("Megaamperes unit symbol", "MA");
    case UnitGigaAmpere:
        return i18nc("Gigaamperes unit symbol", "GA");
    case UnitTeraAmpere:
        return i18nc("Teraamperes unit symbol", "TA");
    case UnitPetaAmpere:
        return i18nc("Petaamperes unit symbol", "PA");

    case UnitPercent:
        return i18nc("Percent unit", "%");
    case UnitRpm:
        return i18nc("Revolutions per minute unit symbol", "RPM");
    case UnitCelsius:
        return i18nc("Celsius unit symbol", "°C");
    case UnitDecibelMilliWatts:
        return i18nc("Decibels unit symbol", "dBm");
    case UnitSecond:
        return i18nc("Seconds unit symbol", "s");

    case UnitRate:
        return i18nc("Rate unit symbol", "s⁻¹");

    default:
        return QString();
    }
}

qreal Formatter::maximumLength(Unit unit, const QFont &font)
{
    auto order = unitOrder(unit);

    QString maximum;
    switch (unitBase(unit)) {
    case UnitByte:
        maximum = formatValue(order - 0.5, UnitMegaByte, MetricPrefixMega);
        break;
    case UnitByteRate:
        maximum = formatValue(order - 0.5, UnitMegaByteRate, MetricPrefixMega);
        break;
    case UnitBitRate:
        maximum = formatValue(order - 0.5, UnitMegaBitRate, MetricPrefixMega);
        break;
    case UnitHertz:
        maximum = formatValue(order - 0.5, UnitMegaHertz, MetricPrefixMega);
        break;
    case UnitPercent:
        maximum = formatValue(999.9, UnitPercent);
        break;
    default:
        return -1.0;
    }

    auto metrics = QFontMetrics{font};
    return metrics.horizontalAdvance(maximum);
}

} // namespace KSysGuard

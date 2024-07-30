/*
 *   SPDX-FileCopyrightText: 2008-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "unit_p.h"
#include "velocity_p.h"

#include <KLocalizedString>

#include <math.h>

namespace KUnitConversion
{
class BeaufortUnitPrivate : public UnitPrivate
{
public:
    BeaufortUnitPrivate(CategoryId categoryId,
                        UnitId id,
                        qreal multiplier,
                        const QString &symbol,
                        const QString &description,
                        const QString &matchString,
                        const KLocalizedString &symbolString,
                        const KLocalizedString &realString,
                        const KLocalizedString &integerString)
        : UnitPrivate(categoryId, id, multiplier, symbol, description, matchString, symbolString, realString, integerString)
    {
    }

    qreal toDefault(qreal value) const override
    {
        return 0.836 * pow(value, 3.0 / 2.0);
    }

    qreal fromDefault(qreal value) const override
    {
        return pow(value / 0.836, 2.0 / 3.0);
    }
};

UnitCategory Velocity::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(VelocityCategory, i18n("Speed"), i18n("Speed"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (velocity)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(VelocityCategory,
                              MeterPerSecond,
                              1,
                              i18nc("velocity unit symbol", "m/s"),
                              i18nc("unit description in lists", "meters per second"),
                              i18nc("unit synonyms for matching user input", "meter per second;meters per second;m/s;ms"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 meters per second"),
                              ki18ncp("amount in units (integer)", "%1 meter per second", "%1 meters per second")));

    d->addCommonUnit(UnitPrivate::makeUnit(VelocityCategory,
                             KilometerPerHour,
                             0.277778,
                             i18nc("velocity unit symbol", "km/h"),
                             i18nc("unit description in lists", "kilometers per hour"),
                             i18nc("unit synonyms for matching user input", "kilometer per hour;kilometers per hour;km/h;kmh;kph"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 kilometers per hour"),
                             ki18ncp("amount in units (integer)", "%1 kilometer per hour", "%1 kilometers per hour")));

    d->addCommonUnit(UnitPrivate::makeUnit(VelocityCategory,
                             MilePerHour,
                             0.44704,
                             i18nc("velocity unit symbol", "mph"),
                             i18nc("unit description in lists", "miles per hour"),
                             i18nc("unit synonyms for matching user input", "mile per hour;miles per hour;mph"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 miles per hour"),
                             ki18ncp("amount in units (integer)", "%1 mile per hour", "%1 miles per hour")));

    d->addUnit(UnitPrivate::makeUnit(VelocityCategory,
                       FootPerSecond,
                       0.3048,
                       i18nc("velocity unit symbol", "ft/s"),
                       i18nc("unit description in lists", "feet per second"),
                       i18nc("unit synonyms for matching user input", "foot per second;feet per second;ft/s;ft/sec;fps"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 feet per second"),
                       ki18ncp("amount in units (integer)", "%1 foot per second", "%1 feet per second")));

    d->addUnit(UnitPrivate::makeUnit(VelocityCategory,
                       InchPerSecond,
                       0.0254,
                       i18nc("velocity unit symbol", "in/s"),
                       i18nc("unit description in lists", "inches per second"),
                       i18nc("unit synonyms for matching user input", "inch per second;inches per second;in/s;in/sec;ips"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 inches per second"),
                       ki18ncp("amount in units (integer)", "%1 inch per second", "%1 inches per second")));

    d->addCommonUnit(UnitPrivate::makeUnit(VelocityCategory,
                             Knot,
                             0.514444,
                             i18nc("velocity unit symbol", "kt"),
                             i18nc("unit description in lists", "knots"),
                             i18nc("unit synonyms for matching user input", "knot;knots;kt;nautical miles per hour"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 knots"),
                             ki18ncp("amount in units (integer)", "%1 knot", "%1 knots")));

    // http://en.wikipedia.org/wiki/Speed_of_sound
    d->addCommonUnit(UnitPrivate::makeUnit(VelocityCategory,
                             Mach,
                             343,
                             i18nc("velocity unit symbol", "Ma"),
                             i18nc("unit description in lists", "Mach"),
                             i18nc("unit synonyms for matching user input", "mach;machs;Ma;speed of sound"),
                             symbolString,
                             ki18nc("amount in units (real)", "Mach %1"),
                             ki18ncp("amount in units (integer)", "Mach %1", "Mach %1")));

    d->addUnit(UnitPrivate::makeUnit(VelocityCategory,
                       SpeedOfLight,
                       2.99792458e+08,
                       i18nc("velocity unit symbol", "c"),
                       i18nc("unit description in lists", "speed of light"),
                       i18nc("unit synonyms for matching user input", "speed of light;c"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 speed of light"),
                       ki18ncp("amount in units (integer)", "%1 speed of light", "%1 speed of light")));

    // http://en.wikipedia.org/wiki/Beaufort_scale
    d->addUnit(UnitPrivate::makeUnit(new BeaufortUnitPrivate(VelocityCategory,
                                               Beaufort,
                                               1.0,
                                               i18nc("velocity unit symbol", "bft"),
                                               i18nc("unit description in lists", "Beaufort"),
                                               i18nc("unit synonyms for matching user input", "Beaufort;Bft"),
                                               symbolString,
                                               ki18nc("amount in units (real)", "%1 on the Beaufort scale"),
                                               ki18ncp("amount in units (integer)", "%1 on the Beaufort scale", "%1 on the Beaufort scale"))));

    return c;
}

} // KUnitConversion namespace

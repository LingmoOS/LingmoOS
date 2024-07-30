/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "angle_p.h"
#include "unit_p.h"

#include <KLocalizedString>

#include <math.h>

namespace KUnitConversion
{
class RadiansUnitPrivate : public UnitPrivate
{
public:
    RadiansUnitPrivate(CategoryId categoryId,
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
        return (value / (2 * M_PI)) * 360.0;
    }
    qreal fromDefault(qreal value) const override
    {
        return (value / 360.0) * (2 * M_PI);
    }
};

UnitCategory Angle::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(AngleCategory, i18n("Angle"), i18n("Angle"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (angle)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(AngleCategory,
                              Degree,
                              1,
                              i18nc("angle unit symbol", "°"),
                              i18nc("unit description in lists", "degrees"),
                              i18nc("unit synonyms for matching user input", "deg;degree;degrees;°"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 degrees"),
                              ki18ncp("amount in units (integer)", "%1 degree", "%1 degrees")));

    d->addCommonUnit(UnitPrivate::makeUnit(new RadiansUnitPrivate(AngleCategory,
                                                    Radian,
                                                    1.0,
                                                    i18nc("angle unit symbol", "rad"),
                                                    i18nc("unit description in lists", "radians"),
                                                    i18nc("unit synonyms for matching user input", "rad;radian;radians"),
                                                    symbolString,
                                                    ki18nc("amount in units (real)", "%1 radians"),
                                                    ki18ncp("amount in units (integer)", "%1 radian", "%1 radians"))));

    d->addUnit(UnitPrivate::makeUnit(AngleCategory,
                       Gradian,
                       360.0 / 400.0,
                       i18nc("angle unit symbol", "grad"),
                       i18nc("unit description in lists", "gradians"),
                       i18nc("unit synonyms for matching user input", "grad;gradian;gradians;grade;gon"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 gradians"),
                       ki18ncp("amount in units (integer)", "%1 gradian", "%1 gradians")));

    d->addCommonUnit(UnitPrivate::makeUnit(AngleCategory,
                             ArcMinute,
                             1.0 / 60.0,
                             i18nc("angle unit symbol", "'"),
                             i18nc("unit description in lists", "arc minutes"),
                             i18nc("unit synonyms for matching user input", "minute of arc;MOA;arcminute;minute;'"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 arc minutes"),
                             ki18ncp("amount in units (integer)", "%1 arc minute", "%1 arc minutes")));

    d->addCommonUnit(UnitPrivate::makeUnit(AngleCategory,
                             ArcSecond,
                             1.0 / 3600.0,
                             i18nc("angle unit symbol", "\""),
                             i18nc("unit description in lists", "arc seconds"),
                             i18nc("unit synonyms for matching user input", "second of arc;arcsecond;second;\""),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 arc seconds"),
                             ki18ncp("amount in units (integer)", "%1 arc second", "%1 arc seconds")));

    return c;
}

} // KUnitConversion namespace

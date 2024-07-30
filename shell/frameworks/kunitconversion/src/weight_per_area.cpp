/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *   SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "weight_per_area_p.h"
#include "unit.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory WeightPerArea::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(WeightPerAreaCategory, i18n("Weight per Area"), i18n("Weight per Area"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (weight per area)", "%1 %2");

    d->addCommonUnit(UnitPrivate::makeUnit(AreaCategory,
                                     GramsPerSquareMeter,
                                     1,
                                     i18nc("area unit symbol", "g/m²"),
                                     i18nc("unit description in lists", "grams per square meter"),
                                     i18nc("unit synonyms for matching user input", "gsm;g/m;g/m2;gm2;grams per meter;grams per square meter;grams per meter²"),
                                     symbolString,
                                     ki18nc("amount in units (real)", "%1 grams per square meter"),
                                     ki18ncp("amount in units (integer)", "%1 grams per square meter", "%1 grams per square meter")));

    // source: https://www.ginifab.com/feeds/ozyd2_gm2/
    d->addCommonUnit(UnitPrivate::makeUnit(AreaCategory,
                                           OuncesPerSquareYard,
                                           33.906,
                                           i18nc("area unit symbol", "oz/yd²"),
                                           i18nc("unit description in lists", "ounces per square yard"),
                                           i18nc("unit synonyms for matching user input", "ozyd;ozyd2;oz/yd;oz/yd2;ounces per yard;ounces per square yard;ounces per yard²"),
                                           symbolString,
                                           ki18nc("amount in units (real)", "%1 ounces per square yard"),
                                           ki18ncp("amount in units (integer)", "%1 ounces per square yard", "%1 ounces per square yard")));

    return c;
}

} // KUnitConversion namespace

/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "thermal_generation_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory ThermalGeneration::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(ThermalGenerationCategory, i18n("Thermal Generation"), i18n("Thermal Generation"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (thermal generation)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(ThermalGenerationCategory,
                              WattPerCubicMeter,
                              1,
                              i18nc("thermal generation unit symbol", "W/m³"),
                              i18nc("unit description in lists", "watt per cubic meter"),
                              i18nc("unit synonyms for matching user input", "watt per cubic meter;W/m3;W/m^3"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 watts per cubic meter"),
                              ki18ncp("amount in units (integer)", "%1 watt per cubic meter", "%1 watts per cubic meter")));

    d->addCommonUnit(UnitPrivate::makeUnit(ThermalGenerationCategory,
                             BtuPerHourPerCubicFoot,
                             0.09662,
                             i18nc("thermal generation unit symbol", "Btu/hr/ft³"),
                             i18nc("unit description in lists", "btu per hour per cubic foot"),
                             i18nc("unit synonyms for matching user input", "btu per hour per cubic foot;Btu/hr/ft3;Btu/hr/ft^3;Btu/ft^3/hr;Btu/ft3/hr"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 btu per hour per cubic foot"),
                             ki18ncp("amount in units (integer)", "%1 btu per hour per cubic foot", "%1 btu per hour per cubic foot")));

    return c;
}

} // KUnitConversion namespace

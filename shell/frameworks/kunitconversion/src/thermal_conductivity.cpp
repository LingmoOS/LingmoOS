/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "thermal_conductivity_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory ThermalConductivity::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(ThermalConductivityCategory, i18n("Thermal Conductivity"), i18n("Thermal Conductivity"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (thermal conductivity)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(ThermalConductivityCategory,
                              WattPerMeterKelvin,
                              1,
                              i18nc("thermal conductivity unit symbol", "W/m·K"),
                              i18nc("unit description in lists", "watt per meter kelvin"),
                              i18nc("unit synonyms for matching user input", "watt per meter kelvin;watt per meter-kelvin;W/mK;W/m.K"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 watts per meter kelvin"),
                              ki18ncp("amount in units (integer)", "%1 watt per meter kelvin", "%1 watts per meter kelvin")));

    d->addCommonUnit(UnitPrivate::makeUnit(ThermalConductivityCategory,
                             BtuPerFootHourFahrenheit,
                             0.5779,
                             i18nc("thermal conductivity unit symbol", "Btu/ft·hr·°F"),
                             i18nc("unit description in lists", "btu per foot hour degree Fahrenheit"),
                             i18nc("unit synonyms for matching user input",
                                   "btu per foot hour degree Fahrenheit;btu per foot hour Fahrenheit;btu per foot-hour-Fahrenheit;Btu/ft-hr-F"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 btu per foot hour degree Fahrenheit"),
                             ki18ncp("amount in units (integer)", "%1 btu per foot hour degree Fahrenheit", "%1 btu per foot hour degree Fahrenheit")));

    d->addCommonUnit(UnitPrivate::makeUnit(ThermalConductivityCategory,
                             BtuPerSquareFootHourFahrenheitPerInch,
                             6.9348,
                             i18nc("thermal conductivity unit symbol", "Btu/ft²·hr·°F/in"),
                             i18nc("unit description in lists", "btu per square foot hour degree Fahrenheit per inch"),
                             i18nc("unit synonyms for matching user input",
                                   "btu per square foot hour degree Fahrenheit per inch;btu per foot squared hour Fahrenheit per inch;btu per sq "
                                   "foot-hour-Fahrenheit per inch;Btu/ft^2-hr-F/in"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 btu per square foot hour degree Fahrenheit per inch"),
                             ki18ncp("amount in units (integer)",
                                     "%1 btu per square foot hour degree Fahrenheit per inch",
                                     "%1 btu per square foot hour degree Fahrenheit per inch")));

    return c;
}

} // KUnitConversion namespace

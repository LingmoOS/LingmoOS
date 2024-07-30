/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *   SPDX-FileCopyrightText: 2014 Garret Wassermann <gwasser@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "thermal_flux_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory ThermalFlux::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(ThermalFluxCategory, i18n("Thermal Flux Density"), i18n("Thermal Flux Density"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (thermal flux density)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(ThermalFluxCategory,
                              WattPerSquareMeter,
                              1,
                              i18nc("thermal flux unit symbol", "W/m²"),
                              i18nc("unit description in lists", "watt per square meter"),
                              i18nc("unit synonyms for matching user input", "watt per square meter;W/m2;W/m^2"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 watts per square meter"),
                              ki18ncp("amount in units (integer)", "%1 watt per square meter", "%1 watts per square meter")));

    d->addCommonUnit(UnitPrivate::makeUnit(ThermalFluxCategory,
                             BtuPerHourPerSquareFoot,
                             0.3169986,
                             i18nc("thermal flux unit symbol", "Btu/hr/ft²"),
                             i18nc("unit description in lists", "btu per hour per square foot"),
                             i18nc("unit synonyms for matching user input", "btu per hour per square foot;Btu/hr/ft2;Btu/hr/ft^2;Btu/ft^2/hr;Btu/ft2/hr"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 btu per hour per square foot"),
                             ki18ncp("amount in units (integer)", "%1 btu per hour per square foot", "%1 btu per hour per square foot")));

    return c;
}

} // KUnitConversion namespace

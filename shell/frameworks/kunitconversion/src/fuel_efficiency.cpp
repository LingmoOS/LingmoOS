/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "fuel_efficiency_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
class FuelUnitPrivate : public UnitPrivate
{
public:
    FuelUnitPrivate(CategoryId categoryId,
                    UnitId id,
                    qreal multiplier,
                    const QString &symbol,
                    const QString &description,
                    const QString &matchString,
                    const KLocalizedString &symbolString,
                    const KLocalizedString &realString,
                    const KLocalizedString &integerString,
                    const bool isReciprocalToDefaultUnit = false)
        : UnitPrivate(categoryId, id, multiplier, symbol, description, matchString, symbolString, realString, integerString)
        , m_isReciprocalToDefaultUnit(isReciprocalToDefaultUnit)
    {
    }

    qreal toDefault(qreal value) const override
    {
        if (m_isReciprocalToDefaultUnit) {
            return unitMultiplier() / value;
        }
        return UnitPrivate::toDefault(value);
    }

    qreal fromDefault(qreal value) const override
    {
        if (m_isReciprocalToDefaultUnit) {
            return unitMultiplier() / value;
        }
        return UnitPrivate::fromDefault(value);
    }

private:
    bool m_isReciprocalToDefaultUnit; /* l/100 km (fuel per given distance) is inverse
                                         to MPG and kmpl (distance per given amount of fuel).
                                         Converter has to account for this. */
};

UnitCategory FuelEfficiency::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(FuelEfficiencyCategory, i18n("Fuel Efficiency"), i18n("Fuel Efficiency"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (fuel efficiency)", "%1 %2");

    d->addDefaultUnit(
        UnitPrivate::makeUnit(new FuelUnitPrivate(FuelEfficiencyCategory,
                                       LitersPer100Kilometers,
                                       1,
                                       i18nc("fuelefficiency unit symbol", "l/100 km"),
                                       i18nc("unit description in lists", "liters per 100 kilometers"),
                                       i18nc("unit synonyms for matching user input", "liters per 100 kilometers;liters per 100 kilometers;l/100 km;L/100 km"),
                                       symbolString,
                                       ki18nc("amount in units (real)", "%1 liters per 100 kilometers"),
                                       ki18ncp("amount in units (integer)", "%1 liters per 100 kilometers", "%1 liters per 100 kilometers"))));

    d->addCommonUnit(UnitPrivate::makeUnit(new FuelUnitPrivate(FuelEfficiencyCategory,
                                                 MilePerUsGallon,
                                                 235.215,
                                                 i18nc("fuelefficiency unit symbol", "mpg"),
                                                 i18nc("unit description in lists", "miles per US gallon"),
                                                 i18nc("unit synonyms for matching user input", "mile per US gallon;miles per US gallon;mpg"),
                                                 symbolString,
                                                 ki18nc("amount in units (real)", "%1 miles per US gallon"),
                                                 ki18ncp("amount in units (integer)", "%1 mile per US gallon", "%1 miles per US gallon"),
                                                 true)));

    d->addCommonUnit(UnitPrivate::makeUnit(new FuelUnitPrivate(
        FuelEfficiencyCategory,
        MilePerImperialGallon,
        282.481,
        i18nc("fuelefficiency unit symbol", "mpg (imperial)"),
        i18nc("unit description in lists", "miles per imperial gallon"),
        i18nc("unit synonyms for matching user input", "mile per imperial gallon;miles per imperial gallon;mpg (imperial);imp mpg;mpg (imp)"),
        symbolString,
        ki18nc("amount in units (real)", "%1 miles per imperial gallon"),
        ki18ncp("amount in units (integer)", "%1 mile per imperial gallon", "%1 miles per imperial gallon"),
        true)));

    d->addCommonUnit(UnitPrivate::makeUnit(new FuelUnitPrivate(FuelEfficiencyCategory,
                                                 KilometrePerLitre,
                                                 100.0,
                                                 i18nc("fuelefficiency unit symbol", "kmpl"),
                                                 i18nc("unit description in lists", "kilometers per liter"),
                                                 i18nc("unit synonyms for matching user input", "kilometer per liter;kilometers per liter;kmpl;km/l"),
                                                 symbolString,
                                                 ki18nc("amount in units (real)", "%1 kilometers per liter"),
                                                 ki18ncp("amount in units (integer)", "%1 kilometer per liter", "%1 kilometers per liter"),
                                                 true)));

    return c;
}

} // KUnitConversion namespace

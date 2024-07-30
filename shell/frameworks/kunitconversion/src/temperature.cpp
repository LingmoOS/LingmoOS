/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "temperature_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
class CelsiusUnitPrivate : public UnitPrivate
{
public:
    CelsiusUnitPrivate(CategoryId categoryId,
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
        return value + 273.15;
    }

    qreal fromDefault(qreal value) const override
    {
        return value - 273.15;
    }
};

class FahrenheitUnitPrivate : public UnitPrivate
{
public:
    FahrenheitUnitPrivate(CategoryId categoryId,
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
        return (value + 459.67) * 5.0 / 9.0;
    }

    qreal fromDefault(qreal value) const override
    {
        return (value * 9.0 / 5.0) - 459.67;
    }
};

class DelisleUnitPrivate : public UnitPrivate
{
public:
    DelisleUnitPrivate(CategoryId categoryId,
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
        return 373.15 - (value * 2.0 / 3.0);
    }

    qreal fromDefault(qreal value) const override
    {
        return (373.15 - value) * 3.0 / 2.0;
    }
};

class NewtonUnitPrivate : public UnitPrivate
{
public:
    NewtonUnitPrivate(CategoryId categoryId,
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
        return (value * 100.0 / 33.0) + 273.15;
    }

    qreal fromDefault(qreal value) const override
    {
        return (value - 273.15) * 33.0 / 100.0;
    }
};

class ReaumurUnitPrivate : public UnitPrivate
{
public:
    ReaumurUnitPrivate(CategoryId categoryId,
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
        return (value * 5.0 / 4.0) + 273.15;
    }

    qreal fromDefault(qreal value) const override
    {
        return (value - 273.15) * 4.0 / 5.0;
    }
};

class RomerUnitPrivate : public UnitPrivate
{
public:
    RomerUnitPrivate(CategoryId categoryId,
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
        return (value - 7.5) * 40.0 / 21.0 + 273.15;
    }

    qreal fromDefault(qreal value) const override
    {
        return (value - 273.15) * 21.0 / 40.0 + 7.5;
    }
};

UnitCategory Temperature::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(TemperatureCategory, i18n("Temperature"), i18n("Temperature"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (temperature)", "%1 %2");

    d->addDefaultUnit(UnitPrivate::makeUnit(TemperatureCategory,
                              Kelvin,
                              1,
                              i18nc("temperature unit symbol", "K"),
                              i18nc("unit description in lists", "kelvins"),
                              i18nc("unit synonyms for matching user input", "kelvin;kelvins;K"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 kelvins"),
                              ki18ncp("amount in units (integer)", "%1 kelvin", "%1 kelvins")));

    d->addCommonUnit(UnitPrivate::makeUnit(new CelsiusUnitPrivate(TemperatureCategory,
                                                    Celsius,
                                                    1,
                                                    i18nc("temperature unit symbol", "°C"),
                                                    i18nc("unit description in lists", "Celsius"),
                                                    i18nc("unit synonyms for matching user input", "Celsius;°C;C"),
                                                    symbolString,
                                                    ki18nc("amount in units (real)", "%1 degrees Celsius"),
                                                    ki18ncp("amount in units (integer)", "%1 degree Celsius", "%1 degrees Celsius"))));

    d->addCommonUnit(UnitPrivate::makeUnit(new FahrenheitUnitPrivate(TemperatureCategory,
                                                       Fahrenheit,
                                                       1,
                                                       i18nc("temperature unit symbol", "°F"),
                                                       i18nc("unit description in lists", "Fahrenheit"),
                                                       i18nc("unit synonyms for matching user input", "Fahrenheit;°F;F"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 degrees Fahrenheit"),
                                                       ki18ncp("amount in units (integer)", "%1 degree Fahrenheit", "%1 degrees Fahrenheit"))));

    d->addUnit(UnitPrivate::makeUnit(TemperatureCategory,
                       Rankine,
                       0.555556,
                       i18nc("temperature unit symbol", "R"),
                       i18nc("unit description in lists", "Rankine"),
                       i18nc("unit synonyms for matching user input", "Rankine;°R;R;Ra"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Rankine"),
                       ki18ncp("amount in units (integer)", "%1 Rankine", "%1 Rankine")));

    d->addUnit(UnitPrivate::makeUnit(new DelisleUnitPrivate(TemperatureCategory,
                                              Delisle,
                                              1,
                                              i18nc("temperature unit symbol", "°De"),
                                              i18nc("unit description in lists", "Delisle"),
                                              i18nc("unit synonyms for matching user input", "Delisle;°De;De"),
                                              symbolString,
                                              ki18nc("amount in units (real)", "%1 degrees Delisle"),
                                              ki18ncp("amount in units (integer)", "%1 degree Delisle", "%1 degrees Delisle"))));

    d->addUnit(UnitPrivate::makeUnit(new NewtonUnitPrivate(TemperatureCategory,
                                             TemperatureNewton,
                                             1,
                                             i18nc("temperature unit symbol", "°N"),
                                             i18nc("unit description in lists", "Newton"),
                                             i18nc("unit synonyms for matching user input", "Newton;°N;N"),
                                             symbolString,
                                             ki18nc("amount in units (real)", "%1 degrees Newton"),
                                             ki18ncp("amount in units (integer)", "%1 degree Newton", "%1 degrees Newton"))));

    d->addUnit(UnitPrivate::makeUnit(new ReaumurUnitPrivate(TemperatureCategory,
                                              Reaumur,
                                              1,
                                              i18nc("temperature unit symbol", "°Ré"),
                                              i18nc("unit description in lists", "Réaumur"),
                                              i18nc("unit synonyms for matching user input", "Réaumur;°Ré;Ré;Reaumur;°Re;Re"),
                                              symbolString,
                                              ki18nc("amount in units (real)", "%1 degrees Réaumur"),
                                              ki18ncp("amount in units (integer)", "%1 degree Réaumur", "%1 degrees Réaumur"))));

    d->addUnit(UnitPrivate::makeUnit(new RomerUnitPrivate(TemperatureCategory,
                                            Romer,
                                            1,
                                            i18nc("temperature unit symbol", "°Rø"),
                                            i18nc("unit description in lists", "Rømer"),
                                            i18nc("unit synonyms for matching user input", "Rømer;°Rø;Rø;Romer;°Ro;Ro"),
                                            symbolString,
                                            ki18nc("amount in units (real)", "%1 degrees Rømer"),
                                            ki18ncp("amount in units (integer)", "%1 degree Rømer", "%1 degrees Rømer"))));

    return c;
}

} // KUnitConversion namespace

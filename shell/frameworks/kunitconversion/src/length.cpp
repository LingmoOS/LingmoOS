/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "length_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{

UnitCategory Length::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(LengthCategory, i18n("Length"), i18n("Length"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (length", "%1 %2");

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Yottameter,
                       1e+24,
                       i18nc("length unit symbol", "Ym"),
                       i18nc("unit description in lists", "yottameters"),
                       i18nc("unit synonyms for matching user input", "yottameter;yottameters;Ym"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottameters"),
                       ki18ncp("amount in units (integer)", "%1 yottameter", "%1 yottameters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Zettameter,
                       1e+21,
                       i18nc("length unit symbol", "Zm"),
                       i18nc("unit description in lists", "zettameters"),
                       i18nc("unit synonyms for matching user input", "zettameter;zettameters;Zm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettameters"),
                       ki18ncp("amount in units (integer)", "%1 zettameter", "%1 zettameters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Exameter,
                       1e+18,
                       i18nc("length unit symbol", "Em"),
                       i18nc("unit description in lists", "exameters"),
                       i18nc("unit synonyms for matching user input", "exameter;exameters;Em"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exameters"),
                       ki18ncp("amount in units (integer)", "%1 exameter", "%1 exameters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Petameter,
                       1e+15,
                       i18nc("length unit symbol", "Pm"),
                       i18nc("unit description in lists", "petameters"),
                       i18nc("unit synonyms for matching user input", "petameter;petameters;Pm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petameters"),
                       ki18ncp("amount in units (integer)", "%1 petameter", "%1 petameters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Terameter,
                       1e+12,
                       i18nc("length unit symbol", "Tm"),
                       i18nc("unit description in lists", "terameters"),
                       i18nc("unit synonyms for matching user input", "terameter;terameters;Tm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 terameters"),
                       ki18ncp("amount in units (integer)", "%1 terameter", "%1 terameters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Gigameter,
                       1e+09,
                       i18nc("length unit symbol", "Gm"),
                       i18nc("unit description in lists", "gigameters"),
                       i18nc("unit synonyms for matching user input", "gigameter;gigameters;Gm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 gigameters"),
                       ki18ncp("amount in units (integer)", "%1 gigameter", "%1 gigameters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Megameter,
                       1e+06,
                       i18nc("length unit symbol", "Mm"),
                       i18nc("unit description in lists", "megameters"),
                       i18nc("unit synonyms for matching user input", "megameter;megameters;Mm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 megameters"),
                       ki18ncp("amount in units (integer)", "%1 megameter", "%1 megameters")));

    d->addCommonUnit(UnitPrivate::makeUnit(LengthCategory,
                             Kilometer,
                             1000,
                             i18nc("length unit symbol", "km"),
                             i18nc("unit description in lists", "kilometers"),
                             i18nc("unit synonyms for matching user input", "kilometer;kilometers;km"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 kilometers"),
                             ki18ncp("amount in units (integer)", "%1 kilometer", "%1 kilometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Hectometer,
                       100,
                       i18nc("length unit symbol", "hm"),
                       i18nc("unit description in lists", "hectometers"),
                       i18nc("unit synonyms for matching user input", "hectometer;hectometers;hm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 hectometers"),
                       ki18ncp("amount in units (integer)", "%1 hectometer", "%1 hectometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Decameter,
                       10,
                       i18nc("length unit symbol", "dam"),
                       i18nc("unit description in lists", "decameters"),
                       i18nc("unit synonyms for matching user input", "decameter;decameters;dam"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decameters"),
                       ki18ncp("amount in units (integer)", "%1 decameter", "%1 decameters")));

    d->addDefaultUnit(UnitPrivate::makeUnit(LengthCategory,
                              Meter,
                              1,
                              i18nc("length unit symbol", "m"),
                              i18nc("unit description in lists", "meters"),
                              i18nc("unit synonyms for matching user input", "meter;meters;m"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 meters"),
                              ki18ncp("amount in units (integer)", "%1 meter", "%1 meters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Decimeter,
                       0.1,
                       i18nc("length unit symbol", "dm"),
                       i18nc("unit description in lists", "decimeters"),
                       i18nc("unit synonyms for matching user input", "decimeter;decimeters;dm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decimeters"),
                       ki18ncp("amount in units (integer)", "%1 decimeter", "%1 decimeters")));

    d->addCommonUnit(UnitPrivate::makeUnit(LengthCategory,
                             Centimeter,
                             0.01,
                             i18nc("length unit symbol", "cm"),
                             i18nc("unit description in lists", "centimeters"),
                             i18nc("unit synonyms for matching user input", "centimeter;centimeters;cm"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 centimeters"),
                             ki18ncp("amount in units (integer)", "%1 centimeter", "%1 centimeters")));

    d->addCommonUnit(UnitPrivate::makeUnit(LengthCategory,
                             Millimeter,
                             0.001,
                             i18nc("length unit symbol", "mm"),
                             i18nc("unit description in lists", "millimeters"),
                             i18nc("unit synonyms for matching user input", "millimeter;millimeters;mm"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 millimeters"),
                             ki18ncp("amount in units (integer)", "%1 millimeter", "%1 millimeters")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Micrometer,
                       1e-06,
                       i18nc("length unit symbol", "µm"),
                       i18nc("unit description in lists", "micrometers"),
                       i18nc("unit synonyms for matching user input", "micrometer;micrometers;µm;um"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 micrometers"),
                       ki18ncp("amount in units (integer)", "%1 micrometer", "%1 micrometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Nanometer,
                       1e-09,
                       i18nc("length unit symbol", "nm"),
                       i18nc("unit description in lists", "nanometers"),
                       i18nc("unit synonyms for matching user input", "nanometer;nanometers;nm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 nanometers"),
                       ki18ncp("amount in units (integer)", "%1 nanometer", "%1 nanometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Angstrom,
                       1e-10,
                       i18nc("length unit symbol", "Å"),
                       i18nc("unit description in lists", "Ångström"),
                       i18nc("unit synonyms for matching user input", "Ångström;Ångstrom;Angström;Angstrom;Ångströms;Ångstroms;Angströms;Angstroms;Å"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Ångströms"),
                       ki18ncp("amount in units (integer)", "%1 Ångström", "%1 Ångströms")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Picometer,
                       1e-12,
                       i18nc("length unit symbol", "pm"),
                       i18nc("unit description in lists", "picometers"),
                       i18nc("unit synonyms for matching user input", "picometer;picometers;pm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 picometers"),
                       ki18ncp("amount in units (integer)", "%1 picometer", "%1 picometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Femtometer,
                       1e-15,
                       i18nc("length unit symbol", "fm"),
                       i18nc("unit description in lists", "femtometers"),
                       i18nc("unit synonyms for matching user input", "femtometer;femtometers;fm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 femtometers"),
                       ki18ncp("amount in units (integer)", "%1 femtometer", "%1 femtometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Attometer,
                       1e-18,
                       i18nc("length unit symbol", "am"),
                       i18nc("unit description in lists", "attometers"),
                       i18nc("unit synonyms for matching user input", "attometer;attometers;am"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 attometers"),
                       ki18ncp("amount in units (integer)", "%1 attometer", "%1 attometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Zeptometer,
                       1e-21,
                       i18nc("length unit symbol", "zm"),
                       i18nc("unit description in lists", "zeptometers"),
                       i18nc("unit synonyms for matching user input", "zeptometer;zeptometers;zm"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zeptometers"),
                       ki18ncp("amount in units (integer)", "%1 zeptometer", "%1 zeptometers")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Yoctometer,
                       1e-24,
                       i18nc("length unit symbol", "ym"),
                       i18nc("unit description in lists", "yoctometers"),
                       i18nc("unit synonyms for matching user input", "yoctometer;yoctometers;ym"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yoctometers"),
                       ki18ncp("amount in units (integer)", "%1 yoctometer", "%1 yoctometers")));

    d->addCommonUnit(UnitPrivate::makeUnit(LengthCategory,
                             Inch,
                             0.0254,
                             i18nc("length unit symbol", "in"),
                             i18nc("unit description in lists", "inches"),
                             i18nc("unit synonyms for matching user input", "inch;inches;in;\""),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 inches"),
                             ki18ncp("amount in units (integer)", "%1 inch", "%1 inches")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Thou,
                       0.0000254,
                       i18nc("length unit symbol", "thou"),
                       i18nc("unit description in lists", "thousandths of an inch"),
                       i18nc("unit synonyms for matching user input", "thou;mil;point;thousandth of an inch;thousandths of an inch"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 thousandths of an inch"),
                       ki18ncp("amount in units (integer)", "%1 thousandth of an inch", "%1 thousandths of an inch")));

    d->addCommonUnit(UnitPrivate::makeUnit(LengthCategory,
                             Foot,
                             0.3048,
                             i18nc("length unit symbol", "ft"),
                             i18nc("unit description in lists", "feet"),
                             i18nc("unit synonyms for matching user input", "foot;feet;ft"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 feet"),
                             ki18ncp("amount in units (integer)", "%1 foot", "%1 feet")));

    d->addCommonUnit(UnitPrivate::makeUnit(LengthCategory,
                             Yard,
                             0.9144,
                             i18nc("length unit symbol", "yd"),
                             i18nc("unit description in lists", "yards"),
                             i18nc("unit synonyms for matching user input", "yard;yards;yd"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 yards"),
                             ki18ncp("amount in units (integer)", "%1 yard", "%1 yards")));

    d->addCommonUnit(UnitPrivate::makeUnit(LengthCategory,
                             Mile,
                             1609.34,
                             i18nc("length unit symbol", "mi"),
                             i18nc("unit description in lists", "miles"),
                             i18nc("unit synonyms for matching user input", "mile;miles;mi"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 miles"),
                             ki18ncp("amount in units (integer)", "%1 mile", "%1 miles")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       NauticalMile,
                       1852,
                       i18nc("length unit symbol", "nmi"),
                       i18nc("unit description in lists", "nautical miles"),
                       i18nc("unit synonyms for matching user input", "nautical mile;nautical miles;nmi"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 nautical miles"),
                       ki18ncp("amount in units (integer)", "%1 nautical mile", "%1 nautical miles")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       LightYear,
                       9.46073e+15,
                       i18nc("length unit symbol", "ly"),
                       i18nc("unit description in lists", "light-years"),
                       i18nc("unit synonyms for matching user input", "light-year;light-years;ly;lightyear;lightyears"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 light-years"),
                       ki18ncp("amount in units (integer)", "%1 light-year", "%1 light-years")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       Parsec,
                       3.08568e+16,
                       i18nc("length unit symbol", "pc"),
                       i18nc("unit description in lists", "parsecs"),
                       i18nc("unit synonyms for matching user input", "parsec;parsecs;pc"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 parsecs"),
                       ki18ncp("amount in units (integer)", "%1 parsec", "%1 parsecs")));

    d->addUnit(UnitPrivate::makeUnit(LengthCategory,
                       AstronomicalUnit,
                       1.49598e+11,
                       i18nc("length unit symbol", "au"),
                       i18nc("unit description in lists", "astronomical units"),
                       i18nc("unit synonyms for matching user input", "astronomical unit;astronomical units;au"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 astronomical units"),
                       ki18ncp("amount in units (integer)", "%1 astronomical unit", "%1 astronomical units")));

    return c;
}

} // KUnitConversion namespace

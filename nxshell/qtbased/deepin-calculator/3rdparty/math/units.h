// This file is part of the SpeedCrunch project
// Copyright (C) 2015 Pol Welter <polwelter@gmail.com>
// Copyright (C) 2016 @heldercorreia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#ifndef UNITS_H
#define UNITS_H

#include <QHash>
#include <QMap>

#include "quantity.h"
class Rational;

struct Unit {
    QString name;
    Quantity value;
    Unit(QString name, Quantity value)
        : name(name)
        , value(value)
    { }
    Unit()
        : name("")
        , value(1)
    { }
};

class Units
{
    static void pushUnit(Quantity q, QString name);
    static QHash<QMap<QString, Rational>, Unit> m_matchLookup;
    static QMap<QString, Quantity> m_cache;
    static void initTable();

public:
    static void findUnit(Quantity &q);
    static void clearCache() { m_cache.clear(); }
    static const QList<Unit> getList();

    // Base SI units.
    static const Quantity meter();
    static const Quantity second();
    static const Quantity kilogram();
    static const Quantity ampere();
    static const Quantity mole();
    static const Quantity kelvin();
    static const Quantity candela();

    // Base non-SI units.
    static const Quantity bit();

    // SI prefixes.
    static const Quantity yocto();
    static const Quantity zepto();
    static const Quantity atto();
    static const Quantity femto();
    static const Quantity pico();
    static const Quantity nano();
    static const Quantity micro();
    static const Quantity milli();
    static const Quantity centi();
    static const Quantity deci();

    static const Quantity deca();
    static const Quantity hecto();
    static const Quantity kilo();
    static const Quantity mega();
    static const Quantity giga();
    static const Quantity tera();
    static const Quantity peta();
    static const Quantity exa();
    static const Quantity zetta();
    static const Quantity yotta();

    // Binary prefixes.
    static const Quantity kibi();
    static const Quantity mebi();
    static const Quantity gibi();
    static const Quantity tebi();
    static const Quantity pebi();
    static const Quantity exbi();
    static const Quantity zebi();
    static const Quantity yobi();

    // Derived SI units.
    static const Quantity sqmeter();
    static const Quantity cbmeter();
    static const Quantity newton();
    static const Quantity hertz();
    static const Quantity steradian();
    static const Quantity pascal();
    static const Quantity joule();
    static const Quantity watt();
    static const Quantity coulomb();
    static const Quantity volt();
    static const Quantity farad();
    static const Quantity ohm();
    static const Quantity siemens();
    static const Quantity weber();
    static const Quantity tesla();
    static const Quantity henry();
    static const Quantity lumen();
    static const Quantity lux();
    static const Quantity becquerel();
    static const Quantity gray();
    static const Quantity sievert();
    static const Quantity katal();

    // Derived from SI units.

    // Mass.
    static const Quantity metric_ton();
    static const Quantity short_ton();
    static const Quantity long_ton();
    static const Quantity pound();
    static const Quantity ounce();
    static const Quantity grain();
    static const Quantity gram();
    static const Quantity atomic_mass_unit();
    static const Quantity carat();


    // Distance/length.
    static const Quantity micron();
    static const Quantity angstrom();
    static const Quantity astronomical_unit();
    static const Quantity lightyear();
    static const Quantity lightsecond();
    static const Quantity lightminute();
    static const Quantity parsec();
    // US measures.
    static const Quantity inch();
    static const Quantity foot();
    static const Quantity yard();
    static const Quantity mile();
    static const Quantity rod();
    static const Quantity furlong();
    // Nautical (US).
    static const Quantity fathom();
    static const Quantity nautical_mile();
    static const Quantity cable();

    // Area.
    static const Quantity are();
    static const Quantity hectare();
    static const Quantity acre();

    // Volume.
    static const Quantity US_gallon();
    static const Quantity UK_gallon();
    static const Quantity US_quart();
    static const Quantity UK_quart();
    static const Quantity US_pint();
    static const Quantity UK_pint();
    static const Quantity US_fluid_ounce();
    static const Quantity UK_fluid_ounce();
    static const Quantity liter();

    // Time.
    static const Quantity minute();
    static const Quantity hour();
    static const Quantity day();
    static const Quantity week();
    static const Quantity julian_year();
    static const Quantity tropical_year();
    static const Quantity sidereal_year();

    // Concentration.
    static const Quantity percent();
    static const Quantity ppm();
    static const Quantity ppb();
    static const Quantity karat();

    // Pressure.
    static const Quantity bar();
    static const Quantity atmosphere();
    static const Quantity torr();
    static const Quantity pounds_per_sqinch();

    // Energy.
    static const Quantity electron_volt();
    static const Quantity calorie();
    static const Quantity british_thermal_unit();


    // Information.
    static const Quantity nat();
    static const Quantity hartley();
    static const Quantity byte();


    // Cooking.
    // Note: these again differ from US to UK, Australia, Japan, ...
    // Since for cooking generally not that high a precision is
    // required, let's just stick with the so called 'legal' variant.
    static const Quantity tablespoon();
    static const Quantity teaspoon();
    static const Quantity cup();

    // Various others.
    // TODO: Some of these are constants that should be moved once constants are also accessible via builtin names.
    static const Quantity gravity();
    static const Quantity speed_of_light();
    static const Quantity elementary_charge();
    static const Quantity speed_of_sound_STP();
    static const Quantity knot();
    static const Quantity horsepower();
};

#endif // UNITS_H

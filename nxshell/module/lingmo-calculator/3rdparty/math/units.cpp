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

#include "units.h"

#include "quantity.h"
#include "rational.h"

#include <QString>
#include <QStringList>

#define UNIT_CACHE(name, value) \
    const Quantity Units::name() \
    { \
        if (!m_cache.contains(#name)) \
            m_cache[#name] = value; \
        return (m_cache[#name]); \
    }

#define BASE_UNIT_CACHE(name, dimension) \
    const Quantity Units::name() \
    { \
        if (!m_cache.contains(#name)) { \
            Quantity name(1); \
            name.modifyDimension(dimension, 1); \
            m_cache[#name] = name; \
        } \
        return m_cache[#name]; \
    }

QHash<QMap<QString, Rational>, Unit> Units::m_matchLookup;
QMap<QString, Quantity> Units::m_cache;


void Units::pushUnit(Quantity q, QString name)
{
    q.cleanDimension();
    Unit u(name, q);
    m_matchLookup.insert(q.getDimension(), u);
}

unsigned int qHash(QMap<QString, Rational> dimension)
{
    QStringList keyList(dimension.keys());
    QString blob("");
    keyList.sort();
    for (int i = 0; i < keyList.size(); ++i) {
        keyList[i].append(dimension[keyList[i]].toString());
        blob.append(keyList[i]);
    }
    return qHash(blob);
}

/*
 * initialize the lookup table for automatic matching
 */
void Units::initTable()
{
    m_matchLookup.clear();
    pushUnit(joule(), "newton meter");                          // energy or torque
    pushUnit(newton(), "newton");                               // force
    pushUnit(watt(), "watt");                                   // power
    pushUnit(pascal(), "pascal");                               // pressure or energy density
    pushUnit(coulomb(), "coulomb");                             // charge
    pushUnit(volt(), "volt");                                   // electrical potential
    pushUnit(ohm(), "ohm");                                     // el. resistance
    pushUnit(siemens(), "siemens");                             // el. conductance
    pushUnit(ohm()*meter(), "ohm meter");                       // el. resistivity
    pushUnit(siemens() / meter(), "siemens/meter");             // el. conductivity
    pushUnit(siemens() / meter() / mole(), "siemens/(meter mole)"); // molar conductivity
    pushUnit(farad(), "farad");                                 // capacitance
    pushUnit(tesla(), "tesla");                                 // magnetic flux density
    pushUnit(weber(), "weber");                                 // magnetic flux
    pushUnit(henry(), "henry");                                 // inductance
    pushUnit(coulomb() / cbmeter(), "coulomb/meter³");          // electric charge density
    pushUnit(coulomb() / sqmeter(), "coulomb/meter²");          // surface charge density or el. flux
    pushUnit(coulomb() / kilogram(), "coulomb/kilogram");       // exposure
    pushUnit(farad() / meter(), "farad/meter");                 // permittivity
    pushUnit(henry() / meter(), "henry/meter");                 // permeability
    pushUnit(joule() / kilogram() / kelvin(), "joule/(kilogram kelvin)"); // specific heat capacity
    pushUnit(joule() / mole() / kelvin(), "joule/(mole kelvin");        // molar heat capacity
    pushUnit(mole() / second() / cbmeter(), "mole/(second meter³)");    // catalytic activity
    pushUnit(newton() / meter(), "newton/meter");               // surface tension
    pushUnit(pascal()*second(), "pascal second");               // dynamic viscosity
    pushUnit(volt() / meter(), "volt/meter");                   // el. field
    pushUnit(watt() / meter() / kelvin(), "watt/(meter kelvin)"); // thermal conductivity
    pushUnit(watt() / sqmeter(), "watt/meter²");                // heat flux
    pushUnit(joule() / kelvin(), "joule/kelvin");               // entropy or heat capacity
    pushUnit(joule() / kilogram(), "joule/kilogram");           // specific energy
}

void Units::findUnit(Quantity &q)
{
    QString unit_name = "";
    CNumber unit(1);
    q.cleanDimension();

    if (m_matchLookup.isEmpty())
        initTable();

    // Match derived units.
    if (m_matchLookup.contains(q.getDimension())) {
        Unit temp(m_matchLookup[q.getDimension()]);
        q.setDisplayUnit(temp.value.numericValue(), temp.name);
    } else {
        // Autogenerate unit name (product of base units).
        auto dimension = q.getDimension();
        auto i = dimension.constBegin();
        while (i != dimension.constEnd()) {
            auto exponent = i.value().toString();
            if (exponent.contains('/'))
                exponent = "^(" + exponent + ')';
            else if (exponent == "1")
                exponent = "";
            else
                exponent = '^' + exponent;

            if (exponent == QLatin1String("^0")) exponent = QString::fromUtf8("⁰");
            else if (exponent == QLatin1String("^2")) exponent = QString::fromUtf8("²");
            else if (exponent == QLatin1String("^3")) exponent = QString::fromUtf8("³");
            else if (exponent == QLatin1String("^4")) exponent = QString::fromUtf8("⁴") ;
            else if (exponent == QLatin1String("^5")) exponent = QString::fromUtf8("⁵") ;
            else if (exponent == QLatin1String("^6")) exponent = QString::fromUtf8("⁶") ;
            else if (exponent == QLatin1String("^7")) exponent = QString::fromUtf8("⁷") ;
            else if (exponent == QLatin1String("^8")) exponent = QString::fromUtf8("⁸") ;
            else if (exponent == QLatin1String("^9")) exponent = QString::fromUtf8("⁹") ;
            else if (exponent == QLatin1String("^-1")) exponent = QString::fromUtf8("⁻¹");
            else if (exponent == QLatin1String("^-2")) exponent = QString::fromUtf8("⁻²");
            else if (exponent == QLatin1String("^-3")) exponent = QString::fromUtf8("⁻³");
            else if (exponent == QLatin1String("^-4")) exponent = QString::fromUtf8("⁻⁴") ;
            else if (exponent == QLatin1String("^-5")) exponent = QString::fromUtf8("⁻⁵") ;
            else if (exponent == QLatin1String("^-6")) exponent = QString::fromUtf8("⁻⁶") ;
            else if (exponent == QLatin1String("^-7")) exponent = QString::fromUtf8("⁻⁷") ;
            else if (exponent == QLatin1String("^-8")) exponent = QString::fromUtf8("⁻⁸") ;
            else if (exponent == QLatin1String("^-9")) exponent = QString::fromUtf8("⁻⁹") ;

            // TODO: Replace this with a lookup to a repository.
            if (i.key() == "length")
                unit_name += " meter";
            else if (i.key() == "time")
                unit_name += " second";
            else if (i.key() == "mass")
                unit_name += " kilogram";
            else if (i.key() == "el. current")
                unit_name += " ampere";
            else if (i.key() == "amount")
                unit_name += " mole";
            else if (i.key() == "luminous intensity")
                unit_name += " candela";
            else if (i.key() == "temperature")
                unit_name += " kelvin";
            else if (i.key() == "information")
                unit_name += " bit";
            else
                unit_name += " " + i.key(); // fall back to the dimension name
            unit_name += exponent;
            ++i;
        }
        q.setDisplayUnit(unit, unit_name.trimmed());
    }
}

#define ADD_UNIT(name) result.append(Unit(#name, name()))
#define ADD_UNIT_ALIAS(name, alias) result.append(Unit(#alias, name()))

// This list contains the units that wil be set as builtin variables by the evaluator.
const QList<Unit> Units::getList()
{
    QList<Unit> result;

    ADD_UNIT(meter);
    ADD_UNIT(second);
    ADD_UNIT(kilogram);
    ADD_UNIT(ampere);
    ADD_UNIT(mole);
    ADD_UNIT(candela);
    ADD_UNIT(kelvin);
    ADD_UNIT(bit);

    ADD_UNIT(yocto);
    ADD_UNIT(zepto);
    ADD_UNIT(atto);
    ADD_UNIT(femto);
    ADD_UNIT(pico);
    ADD_UNIT(nano);
    ADD_UNIT(micro);
    ADD_UNIT(milli);
    ADD_UNIT(centi);
    ADD_UNIT(deci);
    ADD_UNIT(deca);
    ADD_UNIT(hecto);
    ADD_UNIT(kilo);
    ADD_UNIT(mega);
    ADD_UNIT(giga);
    ADD_UNIT(tera);
    ADD_UNIT(peta);
    ADD_UNIT(exa);
    ADD_UNIT(zetta);
    ADD_UNIT(yotta);
    ADD_UNIT(kibi);
    ADD_UNIT(mebi);
    ADD_UNIT(gibi);
    ADD_UNIT(tebi);
    ADD_UNIT(pebi);
    ADD_UNIT(exbi);
    ADD_UNIT(zebi);
    ADD_UNIT(yobi);

    ADD_UNIT(sqmeter);
    ADD_UNIT(cbmeter);
    ADD_UNIT(newton);
    ADD_UNIT(hertz);
    ADD_UNIT(joule);
    ADD_UNIT(watt);
    ADD_UNIT(pascal);
    ADD_UNIT(coulomb);
    ADD_UNIT(volt);
    ADD_UNIT(ohm);
    ADD_UNIT(farad);
    ADD_UNIT(tesla);
    ADD_UNIT(weber);
    ADD_UNIT(henry);
    ADD_UNIT(siemens);
    ADD_UNIT(becquerel);
    ADD_UNIT(gray);
    ADD_UNIT(sievert);
    ADD_UNIT(katal);
    ADD_UNIT(steradian);
    ADD_UNIT(lumen);
    ADD_UNIT(lux);

    ADD_UNIT(metric_ton);
    ADD_UNIT(short_ton);
    ADD_UNIT(long_ton);
    ADD_UNIT(pound);
    ADD_UNIT(ounce);
    ADD_UNIT(grain);
    ADD_UNIT(gram);
    ADD_UNIT(atomic_mass_unit);
    ADD_UNIT(carat);

    ADD_UNIT(micron);
    ADD_UNIT(angstrom);
    ADD_UNIT(astronomical_unit);
    ADD_UNIT(lightyear);
    ADD_UNIT(lightsecond);
    ADD_UNIT(lightminute);
    ADD_UNIT(parsec);
    ADD_UNIT(inch);
    ADD_UNIT(foot);
    ADD_UNIT(yard);
    ADD_UNIT(mile);
    ADD_UNIT(rod);
    ADD_UNIT(furlong);
    ADD_UNIT(fathom);
    ADD_UNIT(nautical_mile);
    ADD_UNIT(cable);

    ADD_UNIT(UK_gallon);
    ADD_UNIT(US_gallon);
    ADD_UNIT_ALIAS(US_gallon, gallon_US);
    ADD_UNIT_ALIAS(UK_gallon, gallon_UK);
    ADD_UNIT_ALIAS(UK_gallon, imperial_gallon);
    ADD_UNIT(UK_quart);
    ADD_UNIT(US_quart);
    ADD_UNIT_ALIAS(US_quart, quart_US);
    ADD_UNIT_ALIAS(UK_quart, quart_UK);
    ADD_UNIT(UK_pint);
    ADD_UNIT(US_pint);
    ADD_UNIT_ALIAS(US_pint, pint_US);
    ADD_UNIT_ALIAS(UK_pint, pint_UK);
    ADD_UNIT(UK_fluid_ounce);
    ADD_UNIT(US_fluid_ounce);
    ADD_UNIT_ALIAS(US_fluid_ounce, fluid_ounce_US);
    ADD_UNIT_ALIAS(UK_fluid_ounce, fluid_ounce_UK);
    ADD_UNIT(liter);

    ADD_UNIT(minute);
    ADD_UNIT(hour);
    ADD_UNIT(day);
    ADD_UNIT(week);
    ADD_UNIT(julian_year);
    ADD_UNIT(tropical_year);
    ADD_UNIT(sidereal_year);
    ADD_UNIT_ALIAS(julian_year, year_julian);
    ADD_UNIT_ALIAS(tropical_year, year_tropical);
    ADD_UNIT_ALIAS(sidereal_year, year_sidereal);

    ADD_UNIT(percent);
    ADD_UNIT(ppm);
    ADD_UNIT(ppb);
    ADD_UNIT(karat);

    ADD_UNIT(bar);
    ADD_UNIT(atmosphere);
    ADD_UNIT(torr);
    ADD_UNIT(pounds_per_sqinch);

    ADD_UNIT(electron_volt);
    ADD_UNIT(calorie);
    ADD_UNIT(british_thermal_unit);

    ADD_UNIT(nat);
    ADD_UNIT(hartley);
    ADD_UNIT(byte);

    ADD_UNIT(tablespoon);
    ADD_UNIT(teaspoon);
    ADD_UNIT(cup);

    ADD_UNIT(gravity);
    ADD_UNIT_ALIAS(gravity, force);
    ADD_UNIT(speed_of_light);
    ADD_UNIT(speed_of_sound_STP);
    ADD_UNIT(elementary_charge);
    ADD_UNIT(knot);
    ADD_UNIT(horsepower);

    return result;
}

BASE_UNIT_CACHE(meter, "length")
BASE_UNIT_CACHE(second, "time")
BASE_UNIT_CACHE(kilogram, "mass")
BASE_UNIT_CACHE(ampere, "el. current")
BASE_UNIT_CACHE(mole, "amount")
BASE_UNIT_CACHE(kelvin, "temperature")
BASE_UNIT_CACHE(candela, "luminous intensity")
BASE_UNIT_CACHE(bit, "information")

UNIT_CACHE(yocto, HNumber("1e-24"))
UNIT_CACHE(zepto, HNumber("1e-21"))
UNIT_CACHE(atto, HNumber("1e-18"))
UNIT_CACHE(femto, HNumber("1e-15"))
UNIT_CACHE(pico, HNumber("1e-12"))
UNIT_CACHE(nano, HNumber("1e-9"))
UNIT_CACHE(micro, HNumber("1e-6"))
UNIT_CACHE(milli, HNumber("1e-3"))
UNIT_CACHE(centi, HNumber("1e-2"))
UNIT_CACHE(deci, HNumber("1e-1"))

UNIT_CACHE(deca, HNumber("1e1"))
UNIT_CACHE(hecto, HNumber("1e2"))
UNIT_CACHE(kilo, HNumber("1e3"))
UNIT_CACHE(mega, HNumber("1e6"))
UNIT_CACHE(giga, HNumber("1e9"))
UNIT_CACHE(tera, HNumber("1e12"))
UNIT_CACHE(peta, HNumber("1e15"))
UNIT_CACHE(exa, HNumber("1e18"))
UNIT_CACHE(zetta, HNumber("1e21"))
UNIT_CACHE(yotta, HNumber("1e24"))

UNIT_CACHE(kibi, HNumber("1024"))
UNIT_CACHE(mebi, kibi()*kibi())
UNIT_CACHE(gibi, kibi()*mebi())
UNIT_CACHE(tebi, kibi()*gibi())
UNIT_CACHE(pebi, kibi()*tebi())
UNIT_CACHE(exbi, kibi()*pebi())
UNIT_CACHE(zebi, kibi()*exbi())
UNIT_CACHE(yobi, kibi()*zebi())

UNIT_CACHE(newton,              meter() * kilogram() / (second()*second()))
UNIT_CACHE(hertz,               Quantity(1) / second())
UNIT_CACHE(pascal,              newton() / sqmeter())
UNIT_CACHE(joule,               newton() * meter())
UNIT_CACHE(watt,                joule() / second())
UNIT_CACHE(coulomb,             ampere() * second())
UNIT_CACHE(volt,                watt() / ampere())
UNIT_CACHE(farad,               coulomb() / volt())
UNIT_CACHE(ohm,                 volt() / ampere())
UNIT_CACHE(siemens,             ampere() / volt())
UNIT_CACHE(weber,               volt() * second())
UNIT_CACHE(tesla,               weber() / sqmeter())
UNIT_CACHE(henry,               weber() / ampere())
UNIT_CACHE(becquerel,           Quantity(1) / second())
UNIT_CACHE(gray,                joule() / kilogram())
UNIT_CACHE(sievert,             joule() / kilogram())
UNIT_CACHE(katal,               mole() / second())
UNIT_CACHE(steradian,           1)
UNIT_CACHE(lumen,               candela()*steradian())
UNIT_CACHE(lux,                 lumen() / sqmeter())

UNIT_CACHE(sqmeter,             meter() * meter())
UNIT_CACHE(cbmeter,             sqmeter() * meter())

UNIT_CACHE(metric_ton,          mega()*gram())
UNIT_CACHE(gram,                milli()*kilogram())
UNIT_CACHE(pound,               HNumber("0.45359237") * kilogram())
UNIT_CACHE(ounce,               pound() / HNumber(16))
UNIT_CACHE(grain,               pound() / HNumber(7000))
UNIT_CACHE(short_ton,           HNumber(2000) * pound())
UNIT_CACHE(long_ton,            HNumber(2240) * pound())
UNIT_CACHE(atomic_mass_unit,    HNumber("1.660539040e-27") * kilogram()) // http://physics.nist.gov/cgi-bin/cuu/Value?tukg
UNIT_CACHE(carat,               HNumber(200) * milli()*gram()) // Do not confuse with karat below.

UNIT_CACHE(micron,              micro()*meter())
UNIT_CACHE(angstrom,            HNumber("1e-10") * meter())
UNIT_CACHE(astronomical_unit,   HNumber("149597870700") * meter()) // IAU 2012 Resolution B2.
UNIT_CACHE(lightyear,           speed_of_light() * julian_year())
UNIT_CACHE(lightminute,         speed_of_light() * minute())
UNIT_CACHE(lightsecond,         speed_of_light() * second())
UNIT_CACHE(parsec,              HNumber(648000) / HMath::pi() * astronomical_unit()) // IAU 2015 Resolution B2.
UNIT_CACHE(inch,                HNumber("0.0254") * meter()) // International inch.
UNIT_CACHE(foot,                HNumber(12) * inch())
UNIT_CACHE(yard,                HNumber(36) * inch())
UNIT_CACHE(mile,                HNumber(1760) * yard())
UNIT_CACHE(rod,                 HNumber("5.5") * yard())
UNIT_CACHE(furlong,             HNumber(40) * rod())
UNIT_CACHE(fathom,              HNumber(6) * foot())
UNIT_CACHE(nautical_mile,       HNumber("1852") * meter())
UNIT_CACHE(cable,               HNumber("0.1") * nautical_mile())

UNIT_CACHE(are,                 HNumber(100) * sqmeter())
UNIT_CACHE(hectare,             HNumber(100) * are())
UNIT_CACHE(acre,                mile()*mile() / HNumber(640))

UNIT_CACHE(US_gallon,           HNumber("3.785") * liter())
UNIT_CACHE(UK_gallon,           HNumber("4.54609") * liter())
UNIT_CACHE(US_quart,            US_gallon() / HNumber(4))
UNIT_CACHE(UK_quart,            UK_gallon() / HNumber(4))
UNIT_CACHE(US_pint,             US_gallon() / HNumber(8))
UNIT_CACHE(UK_pint,             UK_gallon() / HNumber(8))
UNIT_CACHE(US_fluid_ounce,      US_gallon() / HNumber(128))
UNIT_CACHE(UK_fluid_ounce,      UK_gallon() / HNumber(160))
UNIT_CACHE(liter,               milli() * cbmeter())


UNIT_CACHE(minute,              HNumber(60) * second())
UNIT_CACHE(hour,                HNumber(60) * minute())
UNIT_CACHE(day,                 HNumber(24) * hour())
UNIT_CACHE(week,                HNumber(7) * day())
UNIT_CACHE(julian_year,         HNumber("365.25") * day())
UNIT_CACHE(tropical_year,       HNumber("365.24219") * day()) // Approx.: changes over time due to Earth's precession.
UNIT_CACHE(sidereal_year,       HNumber("365.25636") * day()) // http://hpiers.obspm.fr/eop-pc/models/constants.html

UNIT_CACHE(percent,             HNumber("0.01"))
UNIT_CACHE(ppm,                 HNumber("1e-6"))
UNIT_CACHE(ppb,                 HNumber("1e-9"))
UNIT_CACHE(karat,               Rational(1, 24).toHNumber()) // Do not confuse with carat above.

UNIT_CACHE(bar,                 HNumber("1e5") * pascal())
UNIT_CACHE(atmosphere,          HNumber("1.01325") * bar())
UNIT_CACHE(torr,                atmosphere() / HNumber(760))
UNIT_CACHE(pounds_per_sqinch,   pound() * gravity() / (inch()*inch()))

UNIT_CACHE(electron_volt,       elementary_charge() * volt())
UNIT_CACHE(calorie,             HNumber("4.1868") * joule()) // International Table calorie.
UNIT_CACHE(british_thermal_unit, HNumber("1055.056") * joule()) // International standard ISO 31-4.

UNIT_CACHE(nat,                 bit() / HMath::ln(2))
UNIT_CACHE(hartley,             HMath::ln(10) * nat())
UNIT_CACHE(byte,                HNumber(8) * bit())

UNIT_CACHE(tablespoon,          HNumber(15) * milli()*liter())
UNIT_CACHE(teaspoon,            HNumber(5) * milli()*liter())
UNIT_CACHE(cup,                 HNumber(240) * milli()*liter())

UNIT_CACHE(gravity,             HNumber("9.80665") * newton() / kilogram()) // 3rd CGPM (1901, CR 70).
UNIT_CACHE(speed_of_light,      HNumber(299792458) * meter() / second())
UNIT_CACHE(elementary_charge,   HNumber("1.6021766208e-19") * coulomb()) // http://physics.nist.gov/cgi-bin/cuu/Value?e
UNIT_CACHE(speed_of_sound_STP,  HNumber(331) * meter() / second())
UNIT_CACHE(knot,                nautical_mile() / hour())
UNIT_CACHE(horsepower,          HNumber(550) * foot() * pound() * gravity() / second()) // Imperial horsepower.

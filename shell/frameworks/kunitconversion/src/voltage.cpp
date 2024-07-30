/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *   SPDX-FileCopyrightText: 2014 Garret Wassermann <gwasser@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "unit_p.h"
#include "voltage_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory Voltage::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(VoltageCategory, i18n("Voltage"), i18n("Voltage"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (voltage", "%1 %2");

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Yottavolts,
                       1e+24,
                       i18nc("voltage unit symbol", "YV"),
                       i18nc("unit description in lists", "yottavolts"),
                       i18nc("unit synonyms for matching user input", "yottavolt;yottavolts;YV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottavolts"),
                       ki18ncp("amount in units (integer)", "%1 yottavolt", "%1 yottavolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Zettavolts,
                       1e+21,
                       i18nc("voltage unit symbol", "ZV"),
                       i18nc("unit description in lists", "zettavolts"),
                       i18nc("unit synonyms for matching user input", "zettavolt;zettavolts;ZV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettavolts"),
                       ki18ncp("amount in units (integer)", "%1 zettavolt", "%1 zettavolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Exavolts,
                       1e+18,
                       i18nc("voltage unit symbol", "EV"),
                       i18nc("unit description in lists", "exavolts"),
                       i18nc("unit synonyms for matching user input", "exavolt;exavolts;EV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exavolts"),
                       ki18ncp("amount in units (integer)", "%1 exavolt", "%1 exavolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Petavolts,
                       1e+15,
                       i18nc("voltage unit symbol", "PV"),
                       i18nc("unit description in lists", "petavolts"),
                       i18nc("unit synonyms for matching user input", "petavolt;petavolts;PV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petavolts"),
                       ki18ncp("amount in units (integer)", "%1 petavolt", "%1 petavolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Teravolts,
                       1e+12,
                       i18nc("voltage unit symbol", "TV"),
                       i18nc("unit description in lists", "teravolts"),
                       i18nc("unit synonyms for matching user input", "teravolt;teravolts;TV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 teravolts"),
                       ki18ncp("amount in units (integer)", "%1 teravolt", "%1 teravolts")));

    d->addCommonUnit(UnitPrivate::makeUnit(VoltageCategory,
                             Gigavolts,
                             1e+09,
                             i18nc("voltage unit symbol", "GV"),
                             i18nc("unit description in lists", "gigavolts"),
                             i18nc("unit synonyms for matching user input", "gigavolt;gigavolts;GV"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 gigavolts"),
                             ki18ncp("amount in units (integer)", "%1 gigavolt", "%1 gigavolts")));

    d->addCommonUnit(UnitPrivate::makeUnit(VoltageCategory,
                             Megavolts,
                             1e+06,
                             i18nc("voltage unit symbol", "MV"),
                             i18nc("unit description in lists", "megavolts"),
                             i18nc("unit synonyms for matching user input", "megavolt;megavolts;MV"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 megavolts"),
                             ki18ncp("amount in units (integer)", "%1 megavolt", "%1 megavolts")));

    d->addCommonUnit(UnitPrivate::makeUnit(VoltageCategory,
                             Kilovolts,
                             1000,
                             i18nc("voltage unit symbol", "kV"),
                             i18nc("unit description in lists", "kilovolts"),
                             i18nc("unit synonyms for matching user input", "kilovolt;kilovolts;kV"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 kilovolts"),
                             ki18ncp("amount in units (integer)", "%1 kilovolt", "%1 kilovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Hectovolts,
                       100,
                       i18nc("voltage unit symbol", "hV"),
                       i18nc("unit description in lists", "hectovolts"),
                       i18nc("unit synonyms for matching user input", "hectovolt;hectovolts;hV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 hectovolts"),
                       ki18ncp("amount in units (integer)", "%1 hectovolt", "%1 hectovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Decavolts,
                       10,
                       i18nc("voltage unit symbol", "daV"),
                       i18nc("unit description in lists", "decavolts"),
                       i18nc("unit synonyms for matching user input", "decavolt;decavolts;daV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decavolts"),
                       ki18ncp("amount in units (integer)", "%1 decavolt", "%1 decavolts")));

    d->addDefaultUnit(UnitPrivate::makeUnit(VoltageCategory,
                              Volts,
                              1,
                              i18nc("voltage unit symbol", "V"),
                              i18nc("unit description in lists", "volts"),
                              i18nc("unit synonyms for matching user input", "volt;volts;V"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 volts"),
                              ki18ncp("amount in units (integer)", "%1 volt", "%1 volts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Decivolts,
                       0.1,
                       i18nc("voltage unit symbol", "dV"),
                       i18nc("unit description in lists", "decivolts"),
                       i18nc("unit synonyms for matching user input", "decivolt;decivolts;dV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decivolts"),
                       ki18ncp("amount in units (integer)", "%1 decivolt", "%1 decivolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Centivolts,
                       0.01,
                       i18nc("voltage unit symbol", "cV"),
                       i18nc("unit description in lists", "centivolts"),
                       i18nc("unit synonyms for matching user input", "centivolt;centivolts;cV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 centivolts"),
                       ki18ncp("amount in units (integer)", "%1 centivolt", "%1 centivolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Millivolts,
                       0.001,
                       i18nc("voltage unit symbol", "mV"),
                       i18nc("unit description in lists", "millivolts"),
                       i18nc("unit synonyms for matching user input", "millivolt;millivolts;mV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 millivolts"),
                       ki18ncp("amount in units (integer)", "%1 millivolt", "%1 millivolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Microvolts,
                       1e-06,
                       i18nc("voltage unit symbol", "µV"),
                       i18nc("unit description in lists", "microvolts"),
                       i18nc("unit synonyms for matching user input", "microvolt;microvolts;µV;uV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 microvolts"),
                       ki18ncp("amount in units (integer)", "%1 microvolt", "%1 microvolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Nanovolts,
                       1e-09,
                       i18nc("voltage unit symbol", "nV"),
                       i18nc("unit description in lists", "nanovolts"),
                       i18nc("unit synonyms for matching user input", "nanovolt;nanovolts;nV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 nanovolts"),
                       ki18ncp("amount in units (integer)", "%1 nanovolt", "%1 nanovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Picovolts,
                       1e-12,
                       i18nc("voltage unit symbol", "pV"),
                       i18nc("unit description in lists", "picovolts"),
                       i18nc("unit synonyms for matching user input", "picovolt;picovolts;pV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 picovolts"),
                       ki18ncp("amount in units (integer)", "%1 picovolt", "%1 picovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Femtovolts,
                       1e-15,
                       i18nc("voltage unit symbol", "fV"),
                       i18nc("unit description in lists", "femtovolts"),
                       i18nc("unit synonyms for matching user input", "femtovolt;femtovolts;fV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 femtovolts"),
                       ki18ncp("amount in units (integer)", "%1 femtovolt", "%1 femtovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Attovolts,
                       1e-18,
                       i18nc("voltage unit symbol", "aV"),
                       i18nc("unit description in lists", "attovolts"),
                       i18nc("unit synonyms for matching user input", "attovolt;attovolts;aV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 attovolts"),
                       ki18ncp("amount in units (integer)", "%1 attovolt", "%1 attovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Zeptovolts,
                       1e-21,
                       i18nc("voltage unit symbol", "zV"),
                       i18nc("unit description in lists", "zeptovolts"),
                       i18nc("unit synonyms for matching user input", "zeptovolt;zeptovolts;zV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zeptovolts"),
                       ki18ncp("amount in units (integer)", "%1 zeptovolt", "%1 zeptovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Yoctovolts,
                       1e-24,
                       i18nc("voltage unit symbol", "yV"),
                       i18nc("unit description in lists", "yoctovolts"),
                       i18nc("unit synonyms for matching user input", "yoctovolt;yoctovolts;yV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yoctovolts"),
                       ki18ncp("amount in units (integer)", "%1 yoctovolt", "%1 yoctovolts")));

    d->addUnit(UnitPrivate::makeUnit(VoltageCategory,
                       Statvolts,
                       299.792458,
                       i18nc("voltage unit symbol", "stV"),
                       i18nc("unit description in lists", "statvolts"),
                       i18nc("unit synonyms for matching user input", "statvolt;statvolts;stV"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 statvolts"),
                       ki18ncp("amount in units (integer)", "%1 statvolt", "%1 statvolts")));

    return c;
}

} // KUnitConversion namespace

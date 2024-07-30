/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *   SPDX-FileCopyrightText: 2014 Garret Wassermann <gwasser@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "electrical_resistance_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory ElectricalResistance::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(ElectricalResistanceCategory, i18n("Resistance"), i18n("Resistance"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (electrical resistance", "%1 %2");

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Yottaohms,
                       1e+24,
                       i18nc("electrical resistance unit symbol", "YΩ"),
                       i18nc("unit description in lists", "yottaohms"),
                       i18nc("unit synonyms for matching user input", "yottaohm;yottaohms;YΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottaohms"),
                       ki18ncp("amount in units (integer)", "%1 yottaohm", "%1 yottaohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Zettaohms,
                       1e+21,
                       i18nc("electrical resistance unit symbol", "ZΩ"),
                       i18nc("unit description in lists", "zettaohms"),
                       i18nc("unit synonyms for matching user input", "zettaohm;zettaohms;ZΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettaohms"),
                       ki18ncp("amount in units (integer)", "%1 zettaohm", "%1 zettaohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Exaohms,
                       1e+18,
                       i18nc("electrical resistance unit symbol", "EΩ"),
                       i18nc("unit description in lists", "exaohms"),
                       i18nc("unit synonyms for matching user input", "exaohm;exaohms;EΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exaohms"),
                       ki18ncp("amount in units (integer)", "%1 exaohm", "%1 exaohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Petaohms,
                       1e+15,
                       i18nc("electrical resistance unit symbol", "PΩ"),
                       i18nc("unit description in lists", "petaohms"),
                       i18nc("unit synonyms for matching user input", "petaohm;petaohms;PΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petaohms"),
                       ki18ncp("amount in units (integer)", "%1 petaohm", "%1 petaohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Teraohms,
                       1e+12,
                       i18nc("electrical resistance unit symbol", "TΩ"),
                       i18nc("unit description in lists", "teraohms"),
                       i18nc("unit synonyms for matching user input", "teraohm;teraohms;TΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 teraohms"),
                       ki18ncp("amount in units (integer)", "%1 teraohm", "%1 teraohms")));

    d->addCommonUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                             Gigaohms,
                             1e+09,
                             i18nc("electrical resistance unit symbol", "GΩ"),
                             i18nc("unit description in lists", "gigaohms"),
                             i18nc("unit synonyms for matching user input", "gigaohm;gigaohms;GΩ"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 gigaohms"),
                             ki18ncp("amount in units (integer)", "%1 gigaohm", "%1 gigaohms")));

    d->addCommonUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                             Megaohms,
                             1e+06,
                             i18nc("electrical resistance unit symbol", "MΩ"),
                             i18nc("unit description in lists", "megaohms"),
                             i18nc("unit synonyms for matching user input", "megaohm;megaohms;MΩ"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 megaohms"),
                             ki18ncp("amount in units (integer)", "%1 megaohm", "%1 megaohms")));

    d->addCommonUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                             Kiloohms,
                             1000,
                             i18nc("electrical resistance unit symbol", "kΩ"),
                             i18nc("unit description in lists", "kiloohms"),
                             i18nc("unit synonyms for matching user input", "kiloohm;kiloohms;kΩ"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 kiloohms"),
                             ki18ncp("amount in units (integer)", "%1 kiloohm", "%1 kiloohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Hectoohms,
                       100,
                       i18nc("electrical resistance unit symbol", "hΩ"),
                       i18nc("unit description in lists", "hectoohms"),
                       i18nc("unit synonyms for matching user input", "hectoohm;hectoohms;hΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 hectoohms"),
                       ki18ncp("amount in units (integer)", "%1 hectoohm", "%1 hectoohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Decaohms,
                       10,
                       i18nc("electrical resistance unit symbol", "daΩ"),
                       i18nc("unit description in lists", "decaohms"),
                       i18nc("unit synonyms for matching user input", "decaohm;decaohms;daΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decaohms"),
                       ki18ncp("amount in units (integer)", "%1 decaohm", "%1 decaohms")));

    d->addDefaultUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                              Ohms,
                              1,
                              i18nc("electrical resistance unit symbol", "Ω"),
                              i18nc("unit description in lists", "ohms"),
                              i18nc("unit synonyms for matching user input", "ohm;ohms;Ω"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 ohms"),
                              ki18ncp("amount in units (integer)", "%1 ohm", "%1 ohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Deciohms,
                       0.1,
                       i18nc("electrical resistance unit symbol", "dΩ"),
                       i18nc("unit description in lists", "deciohms"),
                       i18nc("unit synonyms for matching user input", "deciohm;deciohms;dΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 deciohms"),
                       ki18ncp("amount in units (integer)", "%1 deciohm", "%1 deciohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Centiohms,
                       0.01,
                       i18nc("electrical resistance unit symbol", "cΩ"),
                       i18nc("unit description in lists", "centiohms"),
                       i18nc("unit synonyms for matching user input", "centiohm;centiohms;cΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 centiohms"),
                       ki18ncp("amount in units (integer)", "%1 centiohm", "%1 centiohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Milliohms,
                       0.001,
                       i18nc("electrical resistance unit symbol", "mΩ"),
                       i18nc("unit description in lists", "milliohms"),
                       i18nc("unit synonyms for matching user input", "milliohm;milliohms;mΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 milliohms"),
                       ki18ncp("amount in units (integer)", "%1 milliohm", "%1 milliohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Microohms,
                       1e-06,
                       i18nc("electrical resistance unit symbol", "µΩ"),
                       i18nc("unit description in lists", "microohms"),
                       i18nc("unit synonyms for matching user input", "microohm;microohms;µΩ;uΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 microohms"),
                       ki18ncp("amount in units (integer)", "%1 microohm", "%1 microohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Nanoohms,
                       1e-09,
                       i18nc("electrical resistance unit symbol", "nΩ"),
                       i18nc("unit description in lists", "nanoohms"),
                       i18nc("unit synonyms for matching user input", "nanoohm;nanoohms;nΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 nanoohms"),
                       ki18ncp("amount in units (integer)", "%1 nanoohm", "%1 nanoohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Picoohms,
                       1e-12,
                       i18nc("electrical resistance unit symbol", "pΩ"),
                       i18nc("unit description in lists", "picoohms"),
                       i18nc("unit synonyms for matching user input", "picoohm;picoohms;pΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 picoohms"),
                       ki18ncp("amount in units (integer)", "%1 picoohm", "%1 picoohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Femtoohms,
                       1e-15,
                       i18nc("electrical resistance unit symbol", "fΩ"),
                       i18nc("unit description in lists", "femtoohms"),
                       i18nc("unit synonyms for matching user input", "femtoohm;femtoohms;fΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 femtoohms"),
                       ki18ncp("amount in units (integer)", "%1 femtoohm", "%1 femtoohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Attoohms,
                       1e-18,
                       i18nc("electrical resistance unit symbol", "aΩ"),
                       i18nc("unit description in lists", "attoohms"),
                       i18nc("unit synonyms for matching user input", "attoohm;attoohms;aΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 attoohms"),
                       ki18ncp("amount in units (integer)", "%1 attoohm", "%1 attoohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Zeptoohms,
                       1e-21,
                       i18nc("electrical resistance unit symbol", "zΩ"),
                       i18nc("unit description in lists", "zeptoohms"),
                       i18nc("unit synonyms for matching user input", "zeptoohm;zeptoohms;zΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zeptoohms"),
                       ki18ncp("amount in units (integer)", "%1 zeptoohm", "%1 zeptoohms")));

    d->addUnit(UnitPrivate::makeUnit(ElectricalResistanceCategory,
                       Yoctoohms,
                       1e-24,
                       i18nc("electrical resistance unit symbol", "yΩ"),
                       i18nc("unit description in lists", "yoctoohms"),
                       i18nc("unit synonyms for matching user input", "yoctoohm;yoctoohms;yΩ"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yoctoohms"),
                       ki18ncp("amount in units (integer)", "%1 yoctoohm", "%1 yoctoohms")));

    return c;
}

} // KUnitConversion namespace

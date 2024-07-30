/*
 *   SPDX-FileCopyrightText: 2010 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "frequency_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{

UnitCategory Frequency::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(FrequencyCategory, i18n("Frequency"), i18n("Frequency"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (frequency", "%1 %2");

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Yottahertz,
                       1e+24,
                       i18nc("frequency unit symbol", "YHz"),
                       i18nc("unit description in lists", "yottahertzs"),
                       i18nc("unit synonyms for matching user input", "yottahertz;yottahertzs;YHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottahertzs"),
                       ki18ncp("amount in units (integer)", "%1 yottahertz", "%1 yottahertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Zettahertz,
                       1e+21,
                       i18nc("frequency unit symbol", "ZHz"),
                       i18nc("unit description in lists", "zettahertzs"),
                       i18nc("unit synonyms for matching user input", "zettahertz;zettahertzs;ZHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettahertzs"),
                       ki18ncp("amount in units (integer)", "%1 zettahertz", "%1 zettahertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Exahertz,
                       1e+18,
                       i18nc("frequency unit symbol", "EHz"),
                       i18nc("unit description in lists", "exahertzs"),
                       i18nc("unit synonyms for matching user input", "exahertz;exahertzs;EHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exahertzs"),
                       ki18ncp("amount in units (integer)", "%1 exahertz", "%1 exahertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Petahertz,
                       1e+15,
                       i18nc("frequency unit symbol", "PHz"),
                       i18nc("unit description in lists", "petahertzs"),
                       i18nc("unit synonyms for matching user input", "petahertz;petahertzs;PHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petahertzs"),
                       ki18ncp("amount in units (integer)", "%1 petahertz", "%1 petahertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Terahertz,
                       1e+12,
                       i18nc("frequency unit symbol", "THz"),
                       i18nc("unit description in lists", "terahertzs"),
                       i18nc("unit synonyms for matching user input", "terahertz;terahertzs;THz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 terahertzs"),
                       ki18ncp("amount in units (integer)", "%1 terahertz", "%1 terahertzs")));

    d->addCommonUnit(UnitPrivate::makeUnit(FrequencyCategory,
                             Gigahertz,
                             1e+09,
                             i18nc("frequency unit symbol", "GHz"),
                             i18nc("unit description in lists", "gigahertzs"),
                             i18nc("unit synonyms for matching user input", "gigahertz;gigahertzs;GHz"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 gigahertzs"),
                             ki18ncp("amount in units (integer)", "%1 gigahertz", "%1 gigahertzs")));

    d->addCommonUnit(UnitPrivate::makeUnit(FrequencyCategory,
                             Megahertz,
                             1e+06,
                             i18nc("frequency unit symbol", "MHz"),
                             i18nc("unit description in lists", "megahertzs"),
                             i18nc("unit synonyms for matching user input", "megahertz;megahertzs;MHz"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 megahertzs"),
                             ki18ncp("amount in units (integer)", "%1 megahertz", "%1 megahertzs")));

    d->addCommonUnit(UnitPrivate::makeUnit(FrequencyCategory,
                             Kilohertz,
                             1000,
                             i18nc("frequency unit symbol", "kHz"),
                             i18nc("unit description in lists", "kilohertzs"),
                             i18nc("unit synonyms for matching user input", "kilohertz;kilohertzs;kHz"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 kilohertzs"),
                             ki18ncp("amount in units (integer)", "%1 kilohertz", "%1 kilohertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Hectohertz,
                       100,
                       i18nc("frequency unit symbol", "hHz"),
                       i18nc("unit description in lists", "hectohertzs"),
                       i18nc("unit synonyms for matching user input", "hectohertz;hectohertzs;hHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 hectohertzs"),
                       ki18ncp("amount in units (integer)", "%1 hectohertz", "%1 hectohertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Decahertz,
                       10,
                       i18nc("frequency unit symbol", "daHz"),
                       i18nc("unit description in lists", "decahertzs"),
                       i18nc("unit synonyms for matching user input", "decahertz;decahertzs;daHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decahertzs"),
                       ki18ncp("amount in units (integer)", "%1 decahertz", "%1 decahertzs")));

    d->addDefaultUnit(UnitPrivate::makeUnit(FrequencyCategory,
                              Hertz,
                              1,
                              i18nc("frequency unit symbol", "Hz"),
                              i18nc("unit description in lists", "hertzs"),
                              i18nc("unit synonyms for matching user input", "hertz;hertzs;Hz"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 hertzs"),
                              ki18ncp("amount in units (integer)", "%1 hertz", "%1 hertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Decihertz,
                       0.1,
                       i18nc("frequency unit symbol", "dHz"),
                       i18nc("unit description in lists", "decihertzs"),
                       i18nc("unit synonyms for matching user input", "decihertz;decihertzs;dHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decihertzs"),
                       ki18ncp("amount in units (integer)", "%1 decihertz", "%1 decihertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Centihertz,
                       0.01,
                       i18nc("frequency unit symbol", "cHz"),
                       i18nc("unit description in lists", "centihertzs"),
                       i18nc("unit synonyms for matching user input", "centihertz;centihertzs;cHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 centihertzs"),
                       ki18ncp("amount in units (integer)", "%1 centihertz", "%1 centihertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Millihertz,
                       0.001,
                       i18nc("frequency unit symbol", "mHz"),
                       i18nc("unit description in lists", "millihertzs"),
                       i18nc("unit synonyms for matching user input", "millihertz;millihertzs;mHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 millihertzs"),
                       ki18ncp("amount in units (integer)", "%1 millihertz", "%1 millihertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Microhertz,
                       1e-06,
                       i18nc("frequency unit symbol", "µHz"),
                       i18nc("unit description in lists", "microhertzs"),
                       i18nc("unit synonyms for matching user input", "microhertz;microhertzs;µHz;uHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 microhertzs"),
                       ki18ncp("amount in units (integer)", "%1 microhertz", "%1 microhertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Nanohertz,
                       1e-09,
                       i18nc("frequency unit symbol", "nHz"),
                       i18nc("unit description in lists", "nanohertzs"),
                       i18nc("unit synonyms for matching user input", "nanohertz;nanohertzs;nHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 nanohertzs"),
                       ki18ncp("amount in units (integer)", "%1 nanohertz", "%1 nanohertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Picohertz,
                       1e-12,
                       i18nc("frequency unit symbol", "pHz"),
                       i18nc("unit description in lists", "picohertzs"),
                       i18nc("unit synonyms for matching user input", "picohertz;picohertzs;pHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 picohertzs"),
                       ki18ncp("amount in units (integer)", "%1 picohertz", "%1 picohertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Femtohertz,
                       1e-15,
                       i18nc("frequency unit symbol", "fHz"),
                       i18nc("unit description in lists", "femtohertzs"),
                       i18nc("unit synonyms for matching user input", "femtohertz;femtohertzs;fHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 femtohertzs"),
                       ki18ncp("amount in units (integer)", "%1 femtohertz", "%1 femtohertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Attohertz,
                       1e-18,
                       i18nc("frequency unit symbol", "aHz"),
                       i18nc("unit description in lists", "attohertzs"),
                       i18nc("unit synonyms for matching user input", "attohertz;attohertzs;aHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 attohertzs"),
                       ki18ncp("amount in units (integer)", "%1 attohertz", "%1 attohertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Zeptohertz,
                       1e-21,
                       i18nc("frequency unit symbol", "zHz"),
                       i18nc("unit description in lists", "zeptohertzs"),
                       i18nc("unit synonyms for matching user input", "zeptohertz;zeptohertzs;zHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zeptohertzs"),
                       ki18ncp("amount in units (integer)", "%1 zeptohertz", "%1 zeptohertzs")));

    d->addUnit(UnitPrivate::makeUnit(FrequencyCategory,
                       Yoctohertz,
                       1e-24,
                       i18nc("frequency unit symbol", "yHz"),
                       i18nc("unit description in lists", "yoctohertzs"),
                       i18nc("unit synonyms for matching user input", "yoctohertz;yoctohertzs;yHz"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yoctohertzs"),
                       ki18ncp("amount in units (integer)", "%1 yoctohertz", "%1 yoctohertzs")));

    d->addCommonUnit(UnitPrivate::makeUnit(FrequencyCategory,
                             RPM,
                             1.0 / 60.0,
                             i18nc("frequency unit symbol", "RPM"),
                             i18nc("unit description in lists", "revolutions per minute"),
                             i18nc("unit synonyms for matching user input", "revolutions per minute;revolution per minute;RPM"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 revolutions per minute"),
                             ki18ncp("amount in units (integer)", "%1 revolution per minute", "%1 revolutions per minute")));

    return c;
}

} // KUnitConversion namespace

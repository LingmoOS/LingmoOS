/*
 *   SPDX-FileCopyrightText: 2019 Jonathan Rubenstein <jrubcop@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "binary_data_p.h"
#include "unit_p.h"

#include <KLocalizedString>

#include <math.h>

namespace KUnitConversion
{
class BinaryPowerOfTwoUnitPrivate : public UnitPrivate
{
public:
    BinaryPowerOfTwoUnitPrivate(CategoryId categoryId,
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
        return value * pow(2.0, m_multiplier);
    }
    qreal fromDefault(qreal value) const override
    {
        return value / pow(2.0, m_multiplier);
    }
};

UnitCategory BinaryData::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(BinaryDataCategory, i18n("Binary Data"), i18n("Binary Data Size"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (binary data)", "%1 %2");

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Yobibyte,
                                                       83,
                                                       i18nc("binary data unit symbol", "YiB"),
                                                       i18nc("unit description in lists", "yobibytes"),
                                                       i18nc("unit synonyms for matching user input", "YiB;yobibyte;yobibytes"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 yobibytes"),
                                                       ki18ncp("amount in units (integer)", "%1 yobibyte", "%1 yobibytes"))));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Yobibit,
                                                       80,
                                                       i18nc("binary data unit symbol", "Yib"),
                                                       i18nc("unit description in lists", "yobibits"),
                                                       i18nc("unit synonyms for matching user input", "Yib;yobibit;yobibits"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 yobibits"),
                                                       ki18ncp("amount in units (integer)", "%1 yobibit", "%1 yobibits"))));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Yottabyte,
                       8e+24,
                       i18nc("binary data unit symbol", "YB"),
                       i18nc("unit description in lists", "yottabytes"),
                       i18nc("unit synonyms for matching user input", "YB;yottabyte;yottabytes"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottabytes"),
                       ki18ncp("amount in units (integer)", "%1 yottabyte", "%1 yottabytes")));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Yottabit,
                       1e+24,
                       i18nc("binary data unit symbol", "Yb"),
                       i18nc("unit description in lists", "yottabits"),
                       i18nc("unit synonyms for matching user input", "Yb;yottabit;yottabits"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottabits"),
                       ki18ncp("amount in units (integer)", "%1 yottabit", "%1 yottabits")));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Zebibyte,
                                                       73,
                                                       i18nc("binary data unit symbol", "ZiB"),
                                                       i18nc("unit description in lists", "zebibytes"),
                                                       i18nc("unit synonyms for matching user input", "ZiB;zebibyte;zebibytes"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 zebibytes"),
                                                       ki18ncp("amount in units (integer)", "%1 zebibyte", "%1 zebibytes"))));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Zebibit,
                                                       70,
                                                       i18nc("binary data unit symbol", "Zib"),
                                                       i18nc("unit description in lists", "zebibits"),
                                                       i18nc("unit synonyms for matching user input", "Zib;zebibit;zebibits"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 zebibits"),
                                                       ki18ncp("amount in units (integer)", "%1 zebibit", "%1 zebibits"))));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Zettabyte,
                       8e+21,
                       i18nc("binary data unit symbol", "ZB"),
                       i18nc("unit description in lists", "zettabytes"),
                       i18nc("unit synonyms for matching user input", "ZB;zettabyte;zettabytes"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettabytes"),
                       ki18ncp("amount in units (integer)", "%1 zettabyte", "%1 zettabytes")));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Zettabit,
                       1e+21,
                       i18nc("binary data unit symbol", "Zb"),
                       i18nc("unit description in lists", "zettabits"),
                       i18nc("unit synonyms for matching user input", "Zb;zettabit;zettabits"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettabits"),
                       ki18ncp("amount in units (integer)", "%1 zettabit", "%1 zettabits")));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Exbibyte,
                                                       63,
                                                       i18nc("binary data unit symbol", "EiB"),
                                                       i18nc("unit description in lists", "exbibytes"),
                                                       i18nc("unit synonyms for matching user input", "EiB;exbibyte;exbibytes"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 exbibytes"),
                                                       ki18ncp("amount in units (integer)", "%1 exbibyte", "%1 exbibytes"))));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Exbibit,
                                                       60,
                                                       i18nc("binary data unit symbol", "Eib"),
                                                       i18nc("unit description in lists", "exbibits"),
                                                       i18nc("unit synonyms for matching user input", "Eib;exbibit;exbibits"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 exbibits"),
                                                       ki18ncp("amount in units (integer)", "%1 exbibit", "%1 exbibits"))));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Exabyte,
                       8e+18,
                       i18nc("binary data unit symbol", "EB"),
                       i18nc("unit description in lists", "exabytes"),
                       i18nc("unit synonyms for matching user input", "EB;exabyte;exabytes"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exabytes"),
                       ki18ncp("amount in units (integer)", "%1 exabyte", "%1 exabytes")));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Exabit,
                       1e+18,
                       i18nc("binary data unit symbol", "Eb"),
                       i18nc("unit description in lists", "exabits"),
                       i18nc("unit synonyms for matching user input", "Eb;exabit;exabits"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exabits"),
                       ki18ncp("amount in units (integer)", "%1 exabit", "%1 exabits")));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Pebibyte,
                                                       53,
                                                       i18nc("binary data unit symbol", "PiB"),
                                                       i18nc("unit description in lists", "pebibytes"),
                                                       i18nc("unit synonyms for matching user input", "PiB;pebibyte;pebibytes"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 pebibytes"),
                                                       ki18ncp("amount in units (integer)", "%1 pebibyte", "%1 pebibytes"))));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Pebibit,
                                                       50,
                                                       i18nc("binary data unit symbol", "Pib"),
                                                       i18nc("unit description in lists", "pebibits"),
                                                       i18nc("unit synonyms for matching user input", "Pib;pebibit;pebibits"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 pebibits"),
                                                       ki18ncp("amount in units (integer)", "%1 pebibit", "%1 pebibits"))));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Petabyte,
                       8e+15,
                       i18nc("binary data unit symbol", "PB"),
                       i18nc("unit description in lists", "petabytes"),
                       i18nc("unit synonyms for matching user input", "PB;petabyte;petabytes"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petabytes"),
                       ki18ncp("amount in units (integer)", "%1 petabyte", "%1 petabytes")));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                       Petabit,
                       1e+15,
                       i18nc("binary data unit symbol", "Pb"),
                       i18nc("unit description in lists", "petabits"),
                       i18nc("unit synonyms for matching user input", "Pb;petabit;petabits"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petabits"),
                       ki18ncp("amount in units (integer)", "%1 petabit", "%1 petabits")));

    d->addCommonUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                             Tebibyte,
                                                             43,
                                                             i18nc("binary data unit symbol", "TiB"),
                                                             i18nc("unit description in lists", "tebibytes"),
                                                             i18nc("unit synonyms for matching user input", "TiB;tebibyte;tebibytes"),
                                                             symbolString,
                                                             ki18nc("amount in units (real)", "%1 tebibytes"),
                                                             ki18ncp("amount in units (integer)", "%1 tebibyte", "%1 tebibytes"))));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Tebibit,
                                                       40,
                                                       i18nc("binary data unit symbol", "Tib"),
                                                       i18nc("unit description in lists", "tebibits"),
                                                       i18nc("unit synonyms for matching user input", "Tib;tebibit;tebibits"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 tebibits"),
                                                       ki18ncp("amount in units (integer)", "%1 tebibit", "%1 tebibits"))));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                             Terabyte,
                             8e+12,
                             i18nc("binary data unit symbol", "TB"),
                             i18nc("unit description in lists", "terabytes"),
                             i18nc("unit synonyms for matching user input", "TB;terabyte;terabytes"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 terabytes"),
                             ki18ncp("amount in units (integer)", "%1 terabyte", "%1 terabytes")));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                             Terabit,
                             1e+12,
                             i18nc("binary data unit symbol", "Tb"),
                             i18nc("unit description in lists", "terabits"),
                             i18nc("unit synonyms for matching user input", "Tb;terabit;terabits"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 terabits"),
                             ki18ncp("amount in units (integer)", "%1 terabit", "%1 terabits")));

    d->addCommonUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                             Gibibyte,
                                                             33,
                                                             i18nc("binary data unit symbol", "GiB"),
                                                             i18nc("unit description in lists", "gibibytes"),
                                                             i18nc("unit synonyms for matching user input", "GiB;gibibyte;gibibytes"),
                                                             symbolString,
                                                             ki18nc("amount in units (real)", "%1 gibibytes"),
                                                             ki18ncp("amount in units (integer)", "%1 gibibyte", "%1 gibibytes"))));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Gibibit,
                                                       30,
                                                       i18nc("binary data unit symbol", "Gib"),
                                                       i18nc("unit description in lists", "gibibits"),
                                                       i18nc("unit synonyms for matching user input", "Gib;gibibit;gibibits"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 gibibits"),
                                                       ki18ncp("amount in units (integer)", "%1 gibibit", "%1 gibibits"))));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                             Gigabyte,
                             8e+09,
                             i18nc("binary data unit symbol", "GB"),
                             i18nc("unit description in lists", "gigabytes"),
                             i18nc("unit synonyms for matching user input", "GB;gigabyte;gigabytes"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 gigabytes"),
                             ki18ncp("amount in units (integer)", "%1 gigabyte", "%1 gigabytes")));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                             Gigabit,
                             1e+09,
                             i18nc("binary data unit symbol", "Gb"),
                             i18nc("unit description in lists", "gigabits"),
                             i18nc("unit synonyms for matching user input", "Gb;gigabit;gigabits"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 gigabits"),
                             ki18ncp("amount in units (integer)", "%1 gigabit", "%1 gigabits")));

    d->addCommonUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                             Mebibyte,
                                                             23,
                                                             i18nc("binary data unit symbol", "MiB"),
                                                             i18nc("unit description in lists", "mebibytes"),
                                                             i18nc("unit synonyms for matching user input", "MiB;mebibyte;mebibytes"),
                                                             symbolString,
                                                             ki18nc("amount in units (real)", "%1 mebibytes"),
                                                             ki18ncp("amount in units (integer)", "%1 mebibyte", "%1 mebibytes"))));

    d->addUnit(UnitPrivate::makeUnit(new BinaryPowerOfTwoUnitPrivate(BinaryDataCategory,
                                                       Mebibit,
                                                       20,
                                                       i18nc("binary data unit symbol", "Mib"),
                                                       i18nc("unit description in lists", "mebibits"),
                                                       i18nc("unit synonyms for matching user input", "Mib;mebibit;mebibits"),
                                                       symbolString,
                                                       ki18nc("amount in units (real)", "%1 mebibits"),
                                                       ki18ncp("amount in units (integer)", "%1 mebibit", "%1 mebibits"))));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                             Megabyte,
                             8e+06,
                             i18nc("binary data unit symbol", "MB"),
                             i18nc("unit description in lists", "megabytes"),
                             i18nc("unit synonyms for matching user input", "MB;megabyte;megabytes"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 megabytes"),
                             ki18ncp("amount in units (integer)", "%1 megabyte", "%1 megabytes")));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                             Megabit,
                             1e+06,
                             i18nc("binary data unit symbol", "Mb"),
                             i18nc("unit description in lists", "megabits"),
                             i18nc("unit synonyms for matching user input", "Mb;megabit;megabits"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 megabits"),
                             ki18ncp("amount in units (integer)", "%1 megabit", "%1 megabits")));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                                           Kibibyte,
                                           8192,
                                           i18nc("binary data unit symbol", "KiB"),
                                           i18nc("unit description in lists", "kibibytes"),
                                           i18nc("unit synonyms for matching user input", "KiB;kiB;kibibyte;kibibytes"),
                                           symbolString,
                                           ki18nc("amount in units (real)", "%1 kibibytes"),
                                           ki18ncp("amount in units (integer)", "%1 kibibyte", "%1 kibibytes")));

    d->addUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                                     Kibibit,
                                     1024,
                                     i18nc("binary data unit symbol", "Kib"),
                                     i18nc("unit description in lists", "kibibits"),
                                     i18nc("unit synonyms for matching user input", "Kib;kib;kibibit;kibibits"),
                                     symbolString,
                                     ki18nc("amount in units (real)", "%1 kibibits"),
                                     ki18ncp("amount in units (integer)", "%1 kibibit", "%1 kibibits")));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                                           Kilobyte,
                                           8000,
                                           i18nc("binary data unit symbol", "kB"),
                                           i18nc("unit description in lists", "kilobytes"),
                                           i18nc("unit synonyms for matching user input", "kB;KB;kilobyte;kilobytes"),
                                           symbolString,
                                           ki18nc("amount in units (real)", "%1 kilobytes"),
                                           ki18ncp("amount in units (integer)", "%1 kilobyte", "%1 kilobytes")));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                                           Kilobit,
                                           1000,
                                           i18nc("binary data unit symbol", "kb"),
                                           i18nc("unit description in lists", "kilobits"),
                                           i18nc("unit synonyms for matching user input", "kb;Kb;kilobit;kilobits"),
                                           symbolString,
                                           ki18nc("amount in units (real)", "%1 kilobits"),
                                           ki18ncp("amount in units (integer)", "%1 kilobit", "%1 kilobits")));

    d->addCommonUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                             Byte,
                             8,
                             i18nc("binary data unit symbol", "B"),
                             i18nc("unit description in lists", "bytes"),
                             i18nc("unit synonyms for matching user input", "B;byte;bytes"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 bytes"),
                             ki18ncp("amount in units (integer)", "%1 byte", "%1 bytes")));

    d->addDefaultUnit(UnitPrivate::makeUnit(BinaryDataCategory,
                              Bit,
                              1,
                              i18nc("binary data unit symbol", "b"),
                              i18nc("unit description in lists", "bits"),
                              i18nc("unit synonyms for matching user input", "b;bit;bits"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 bits"),
                              ki18ncp("amount in units (integer)", "%1 bit", "%1 bits")));

    return c;
}

} // KUnitConversion namespace

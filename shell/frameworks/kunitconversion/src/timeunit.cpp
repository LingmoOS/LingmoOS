/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "timeunit_p.h"
#include "unit_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitCategory Time::makeCategory()
{
    auto c = UnitCategoryPrivate::makeCategory(TimeCategory, i18n("Time"), i18n("Time"));
    auto d = UnitCategoryPrivate::get(c);
    KLocalizedString symbolString = ki18nc("%1 value, %2 unit symbol (time)", "%1 %2");

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Yottasecond,
                       1e+24,
                       i18nc("time unit symbol", "Ys"),
                       i18nc("unit description in lists", "yottaseconds"),
                       i18nc("unit synonyms for matching user input", "yottasecond;yottaseconds;Ys"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yottaseconds"),
                       ki18ncp("amount in units (integer)", "%1 yottasecond", "%1 yottaseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Zettasecond,
                       1e+21,
                       i18nc("time unit symbol", "Zs"),
                       i18nc("unit description in lists", "zettaseconds"),
                       i18nc("unit synonyms for matching user input", "zettasecond;zettaseconds;Zs"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zettaseconds"),
                       ki18ncp("amount in units (integer)", "%1 zettasecond", "%1 zettaseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Exasecond,
                       1e+18,
                       i18nc("time unit symbol", "Es"),
                       i18nc("unit description in lists", "exaseconds"),
                       i18nc("unit synonyms for matching user input", "exasecond;exaseconds;Es"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 exaseconds"),
                       ki18ncp("amount in units (integer)", "%1 exasecond", "%1 exaseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Petasecond,
                       1e+15,
                       i18nc("time unit symbol", "Ps"),
                       i18nc("unit description in lists", "petaseconds"),
                       i18nc("unit synonyms for matching user input", "petasecond;petaseconds;Ps"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 petaseconds"),
                       ki18ncp("amount in units (integer)", "%1 petasecond", "%1 petaseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Terasecond,
                       1e+12,
                       i18nc("time unit symbol", "Ts"),
                       i18nc("unit description in lists", "teraseconds"),
                       i18nc("unit synonyms for matching user input", "terasecond;teraseconds;Ts"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 teraseconds"),
                       ki18ncp("amount in units (integer)", "%1 terasecond", "%1 teraseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Gigasecond,
                       1e+09,
                       i18nc("time unit symbol", "Gs"),
                       i18nc("unit description in lists", "gigaseconds"),
                       i18nc("unit synonyms for matching user input", "gigasecond;gigaseconds;Gs"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 gigaseconds"),
                       ki18ncp("amount in units (integer)", "%1 gigasecond", "%1 gigaseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Megasecond,
                       1e+06,
                       i18nc("time unit symbol", "Ms"),
                       i18nc("unit description in lists", "megaseconds"),
                       i18nc("unit synonyms for matching user input", "megasecond;megaseconds;Ms"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 megaseconds"),
                       ki18ncp("amount in units (integer)", "%1 megasecond", "%1 megaseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Kilosecond,
                       1000,
                       i18nc("time unit symbol", "ks"),
                       i18nc("unit description in lists", "kiloseconds"),
                       i18nc("unit synonyms for matching user input", "kilosecond;kiloseconds;ks"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 kiloseconds"),
                       ki18ncp("amount in units (integer)", "%1 kilosecond", "%1 kiloseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Hectosecond,
                       100,
                       i18nc("time unit symbol", "hs"),
                       i18nc("unit description in lists", "hectoseconds"),
                       i18nc("unit synonyms for matching user input", "hectosecond;hectoseconds;hs"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 hectoseconds"),
                       ki18ncp("amount in units (integer)", "%1 hectosecond", "%1 hectoseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Decasecond,
                       10,
                       i18nc("time unit symbol", "das"),
                       i18nc("unit description in lists", "decaseconds"),
                       i18nc("unit synonyms for matching user input", "decasecond;decaseconds;das"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 decaseconds"),
                       ki18ncp("amount in units (integer)", "%1 decasecond", "%1 decaseconds")));

    d->addDefaultUnit(UnitPrivate::makeUnit(TimeCategory,
                              Second,
                              1,
                              i18nc("time unit symbol", "s"),
                              i18nc("unit description in lists", "seconds"),
                              i18nc("unit synonyms for matching user input", "second;seconds;s"),
                              symbolString,
                              ki18nc("amount in units (real)", "%1 seconds"),
                              ki18ncp("amount in units (integer)", "%1 second", "%1 seconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Decisecond,
                       0.1,
                       i18nc("time unit symbol", "ds"),
                       i18nc("unit description in lists", "deciseconds"),
                       i18nc("unit synonyms for matching user input", "decisecond;deciseconds;ds"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 deciseconds"),
                       ki18ncp("amount in units (integer)", "%1 decisecond", "%1 deciseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Centisecond,
                       0.01,
                       i18nc("time unit symbol", "cs"),
                       i18nc("unit description in lists", "centiseconds"),
                       i18nc("unit synonyms for matching user input", "centisecond;centiseconds;cs"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 centiseconds"),
                       ki18ncp("amount in units (integer)", "%1 centisecond", "%1 centiseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Millisecond,
                       0.001,
                       i18nc("time unit symbol", "ms"),
                       i18nc("unit description in lists", "milliseconds"),
                       i18nc("unit synonyms for matching user input", "millisecond;milliseconds;ms"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 milliseconds"),
                       ki18ncp("amount in units (integer)", "%1 millisecond", "%1 milliseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Microsecond,
                       1e-06,
                       i18nc("time unit symbol", "µs"),
                       i18nc("unit description in lists", "microseconds"),
                       i18nc("unit synonyms for matching user input", "microsecond;microseconds;µs;us"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 microseconds"),
                       ki18ncp("amount in units (integer)", "%1 microsecond", "%1 microseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Nanosecond,
                       1e-09,
                       i18nc("time unit symbol", "ns"),
                       i18nc("unit description in lists", "nanoseconds"),
                       i18nc("unit synonyms for matching user input", "nanosecond;nanoseconds;ns"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 nanoseconds"),
                       ki18ncp("amount in units (integer)", "%1 nanosecond", "%1 nanoseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Picosecond,
                       1e-12,
                       i18nc("time unit symbol", "ps"),
                       i18nc("unit description in lists", "picoseconds"),
                       i18nc("unit synonyms for matching user input", "picosecond;picoseconds;ps"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 picoseconds"),
                       ki18ncp("amount in units (integer)", "%1 picosecond", "%1 picoseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Femtosecond,
                       1e-15,
                       i18nc("time unit symbol", "fs"),
                       i18nc("unit description in lists", "femtoseconds"),
                       i18nc("unit synonyms for matching user input", "femtosecond;femtoseconds;fs"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 femtoseconds"),
                       ki18ncp("amount in units (integer)", "%1 femtosecond", "%1 femtoseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Attosecond,
                       1e-18,
                       i18nc("time unit symbol", "as"),
                       i18nc("unit description in lists", "attoseconds"),
                       i18nc("unit synonyms for matching user input", "attosecond;attoseconds;as"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 attoseconds"),
                       ki18ncp("amount in units (integer)", "%1 attosecond", "%1 attoseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Zeptosecond,
                       1e-21,
                       i18nc("time unit symbol", "zs"),
                       i18nc("unit description in lists", "zeptoseconds"),
                       i18nc("unit synonyms for matching user input", "zeptosecond;zeptoseconds;zs"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 zeptoseconds"),
                       ki18ncp("amount in units (integer)", "%1 zeptosecond", "%1 zeptoseconds")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       Yoctosecond,
                       1e-24,
                       i18nc("time unit symbol", "ys"),
                       i18nc("unit description in lists", "yoctoseconds"),
                       i18nc("unit synonyms for matching user input", "yoctosecond;yoctoseconds;ys"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 yoctoseconds"),
                       ki18ncp("amount in units (integer)", "%1 yoctosecond", "%1 yoctoseconds")));

    d->addCommonUnit(UnitPrivate::makeUnit(TimeCategory,
                             Minute,
                             60,
                             i18nc("time unit symbol", "min"),
                             i18nc("unit description in lists", "minutes"),
                             i18nc("unit synonyms for matching user input", "minute;minutes;min"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 minutes"),
                             ki18ncp("amount in units (integer)", "%1 minute", "%1 minutes")));

    d->addCommonUnit(UnitPrivate::makeUnit(TimeCategory,
                             Hour,
                             3600,
                             i18nc("time unit symbol", "h"),
                             i18nc("unit description in lists", "hours"),
                             i18nc("unit synonyms for matching user input", "hour;hours;h"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 hours"),
                             ki18ncp("amount in units (integer)", "%1 hour", "%1 hours")));

    d->addCommonUnit(UnitPrivate::makeUnit(TimeCategory,
                             Day,
                             86400,
                             i18nc("time unit symbol", "d"),
                             i18nc("unit description in lists", "days"),
                             i18nc("unit synonyms for matching user input", "day;days;d"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 days"),
                             ki18ncp("amount in units (integer)", "%1 day", "%1 days")));

    d->addCommonUnit(UnitPrivate::makeUnit(TimeCategory,
                             Week,
                             604800,
                             i18nc("time unit symbol", "w"),
                             i18nc("unit description in lists", "weeks"),
                             i18nc("unit synonyms for matching user input", "week;weeks"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 weeks"),
                             ki18ncp("amount in units (integer)", "%1 week", "%1 weeks")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       JulianYear,
                       3.15576e+07,
                       i18nc("time unit symbol", "a"),
                       i18nc("unit description in lists", "Julian years"),
                       i18nc("unit synonyms for matching user input", "Julian year;Julian years;a"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 Julian years"),
                       ki18ncp("amount in units (integer)", "%1 Julian year", "%1 Julian years")));

    d->addUnit(UnitPrivate::makeUnit(TimeCategory,
                       LeapYear,
                       3.16224e+07,
                       i18nc("time unit symbol", "lpy"),
                       i18nc("unit description in lists", "leap years"),
                       i18nc("unit synonyms for matching user input", "leap year;leap years"),
                       symbolString,
                       ki18nc("amount in units (real)", "%1 leap years"),
                       ki18ncp("amount in units (integer)", "%1 leap year", "%1 leap years")));

    // Let's say we call that a normal year
    d->addCommonUnit(UnitPrivate::makeUnit(TimeCategory,
                             Year,
                             3.1536e+07,
                             i18nc("time unit symbol", "y"),
                             i18nc("unit description in lists", "year"),
                             i18nc("unit synonyms for matching user input", "year;years;y"),
                             symbolString,
                             ki18nc("amount in units (real)", "%1 year"),
                             ki18ncp("amount in units (integer)", "%1 year", "%1 years")));

    return c;
}

} // KUnitConversion namespace

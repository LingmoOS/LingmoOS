// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/battery/timeformat.h"

#include "base/locale.h"

using namespace Lingmo::base;

namespace settings::timeformat {

std::string shortTime(int64_t secs)
{
    return Locale::system().toString(DateTime::fromSecondsSinceEpoch(secs).time, Locale::FormatType::Short);
}

std::string shortDayName(int64_t secs)
{
    return Locale::system().toString(DateTime::fromSecondsSinceEpoch(secs).date, "ddd");
}

std::string shortDateTime(int64_t secs)
{
    return Locale::system().toString(DateTime::fromSecondsSinceEpoch(secs), Locale::FormatType::Short);
}

// Days since 1970-01-01 of a date of the proleptic Gregorian calendar.
static int64_t dayNumber(const CalendarDate& date)
{
    const int64_t year = date.year - (date.month <= 2);
    const int64_t era = (year >= 0 ? year : year - 399) / 400;
    const int64_t yearOfEra = year - era * 400;
    const int64_t dayOfYear = (153 * (date.month + (date.month > 2 ? -3 : 9)) + 2) / 5 + date.day - 1;
    const int64_t dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return era * 146097 + dayOfEra - 719468;
}

int64_t daysBetween(int64_t from, int64_t to)
{
    return dayNumber(DateTime::fromSecondsSinceEpoch(to).date) - dayNumber(DateTime::fromSecondsSinceEpoch(from).date);
}

int64_t utcOffset(int64_t secs)
{
    return DateTime::fromSecondsSinceEpoch(secs).utcOffset;
}

bool isStartOfDay(int64_t secs)
{
    const ClockTime time = DateTime::fromSecondsSinceEpoch(secs).time;
    return time.hour == 0 && time.minute == 0;
}

} // namespace settings::timeformat

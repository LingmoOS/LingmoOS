// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/dates.h"

#include "base/i18n.h"
#include "base/locale.h"

#include <chrono>
#include <cmath>

namespace fm::dates {

using Lingmo::base::DateTime;
using Lingmo::base::Locale;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

static int64_t nowMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

static std::string shortDateTime(int64_t milliseconds)
{
    const Locale& locale = Locale::system();
    return locale.toString(DateTime::fromSecondsSinceEpoch(milliseconds / 1000), Locale::FormatType::Short);
}

std::string friendlyTime(int64_t milliseconds)
{
    const int64_t minutes = std::llround((nowMilliseconds() - milliseconds) / 1000 / 60.0);
    if (minutes < 1)
        return tr("Now");
    if (minutes < 60)
        return trn("{n} minute ago", "{n} minutes ago", uint64_t(minutes));

    const int64_t hours = std::llround(minutes / 60.0);
    if (hours < 24)
        return trn("{n} hour ago", "{n} hours ago", uint64_t(hours));

    const int64_t days = std::llround(hours / 24.0);
    if (days <= 10)
        return trn("{n} day ago", "{n} days ago", uint64_t(days));

    return shortDateTime(milliseconds);
}

static int64_t dayNumber(const Lingmo::base::CalendarDate& date)
{
    // Days since 1970-01-01 for the proleptic Gregorian calendar.
    const int y = date.year - (date.month <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const int yoe = y - era * 400;
    const int doy = (153 * (date.month + (date.month > 2 ? -3 : 9)) + 2) / 5 + date.day - 1;
    const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return int64_t(era) * 146097 + doe - 719468;
}

std::string columnTime(int64_t milliseconds)
{
    if (milliseconds <= 0)
        return {};

    const Locale& locale = Locale::system();
    const DateTime time = DateTime::fromSecondsSinceEpoch(milliseconds / 1000);
    const DateTime now = DateTime::fromSecondsSinceEpoch(nowMilliseconds() / 1000);
    const std::string clock = locale.toString(time.time, Locale::FormatType::Short);
    const int64_t days = dayNumber(now.date) - dayNumber(time.date);
    if (days == 0)
        return format(tr("Today at {time}"), {{"time", clock}});
    if (days == 1)
        return format(tr("Yesterday at {time}"), {{"time", clock}});
    return locale.toString(time, Locale::FormatType::Short);
}

} // namespace fm::dates

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

// Moments in Unix seconds, read on the local clock and written in the
// user's time locale, as QLocale's short formats write them.
namespace settings::timeformat {

// "4:00 pm".
std::string shortTime(int64_t secs);
// "Wed".
std::string shortDayName(int64_t secs);
// "3/10/26 2:21 pm".
std::string shortDateTime(int64_t secs);

// Local whole days from the first moment's date to the second's.
int64_t daysBetween(int64_t from, int64_t to);
// How far local time is ahead of UTC at that moment, in seconds.
int64_t utcOffset(int64_t secs);
// Whether the moment is a local midnight.
bool isStartOfDay(int64_t secs);

} // namespace settings::timeformat

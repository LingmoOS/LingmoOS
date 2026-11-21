// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>

namespace Lingmo::Shell::ClockFormat {

// The status bar clock's format, as base::Locale::toString takes it: the
// weekday, abbreviated month, day and time in the order and words CLDR gives
// the locale, without the commas of its written-out form ("Sat Sep 26 1:40
// PM").
std::string pattern(const std::string &localeName, bool twentyFourHour);

// From a CLDR (ICU) pattern; empty when it has a field that format cannot show.
std::string toLocaleFormat(const std::string &icuPattern);

// A base::Locale::toString format with a seconds field outside its quotes.
bool showsSeconds(const std::string &format);

} // namespace Lingmo::Shell::ClockFormat


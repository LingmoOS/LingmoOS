// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace Lingmo::ui {

// Break attributes, one per UTF-16 position, the end included: whether the
// tailored UAX #14 pair table allows a break before the position, and
// whether the code unit there is white space a line wraps at.
struct BreakAttributes
{
    std::vector<uint8_t> lineBreak;
    std::vector<uint8_t> breakableSpace;
};

BreakAttributes breakAttributes(std::u16string_view text);

// Where a line may wrap: a pair table tailoring of UAX #14 rather than ICU's
// rules, with Korean breaking at spaces, and after any breakable white
// space.
// One entry per UTF-16 position, the end included; true allows a break
// before it.
std::vector<uint8_t> lineBreaks(std::u16string_view text);

} // namespace Lingmo::ui

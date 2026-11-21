// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

namespace fm::dates {

// "Now", "5 minutes ago", … up to ten days, then the short date and time.
std::string friendlyTime(int64_t milliseconds);
// "Today at 9:41", "Yesterday at 9:41", else the short date and time; empty
// for no time.
std::string columnTime(int64_t milliseconds);

} // namespace fm::dates

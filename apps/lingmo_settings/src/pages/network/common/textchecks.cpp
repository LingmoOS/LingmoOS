// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/common/textchecks.h"

#include <cctype>

namespace settings::network {

bool isNumberText(std::string_view text, long long maximum)
{
    if (text.empty())
        return true;
    if (text.size() > 10)
        return false;
    long long value = 0;
    for (const char c : text) {
        if (!std::isdigit(static_cast<unsigned char>(c)))
            return false;
        value = value * 10 + (c - '0');
    }
    return value <= maximum;
}

bool isPortText(std::string_view text)
{
    // Empty or 1 to 65535; a larger number cannot be typed at all.
    return text.empty() || (text[0] != '0' && isNumberText(text, 65535));
}

} // namespace settings::network

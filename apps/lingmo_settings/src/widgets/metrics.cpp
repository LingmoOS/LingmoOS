// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/metrics.h"

#include <cmath>

namespace settings::metrics {

// How far each trailing glyph's drawing stops short of its box's end, as a
// share of the box.
static double glyphEndInset(std::string_view name)
{
    if (name == "check")
        return 1.0 / 6;
    if (name == "info")
        return 1.0 / 8;
    if (name == "chevron-right")
        return 5.0 / 16;
    if (name == "chevron-down" || name == "chevron-up")
        return 3.0 / 16;
    if (name == "refresh")
        return 1.0 / 6;
    return 0;
}

double trailingGlyphMargin(std::string_view name, double boxWidth, double padding)
{
    return -std::round(glyphEndInset(name) * boxWidth + padding);
}

} // namespace settings::metrics

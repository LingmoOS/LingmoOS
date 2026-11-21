// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/geometry.h"

#include <algorithm>
#include <cmath>

namespace Lingmo::ui {

struct Hsv
{
    double h = 0;
    double s = 0;
    double v = 0;
};

static Hsv toHsv(const Color& color)
{
    const double r = color.r / 255.0;
    const double g = color.g / 255.0;
    const double b = color.b / 255.0;
    const double max = std::max({r, g, b});
    const double min = std::min({r, g, b});
    const double delta = max - min;

    Hsv hsv;
    hsv.v = max;
    hsv.s = max > 0 ? delta / max : 0;
    if (delta > 0) {
        if (max == r)
            hsv.h = std::fmod((g - b) / delta, 6.0);
        else if (max == g)
            hsv.h = (b - r) / delta + 2;
        else
            hsv.h = (r - g) / delta + 4;
        hsv.h *= 60;
        if (hsv.h < 0)
            hsv.h += 360;
    }
    return hsv;
}

static Color fromHsv(const Hsv& hsv, uint8_t alpha)
{
    const double c = hsv.v * hsv.s;
    const double x = c * (1 - std::fabs(std::fmod(hsv.h / 60, 2.0) - 1));
    const double m = hsv.v - c;
    double r = 0;
    double g = 0;
    double b = 0;
    switch (int(hsv.h / 60) % 6) {
    case 0:
        r = c, g = x;
        break;
    case 1:
        r = x, g = c;
        break;
    case 2:
        g = c, b = x;
        break;
    case 3:
        g = x, b = c;
        break;
    case 4:
        r = x, b = c;
        break;
    default:
        r = c, b = x;
        break;
    }
    const auto channel = [m](double value) {
        return uint8_t(std::clamp(std::lround((value + m) * 255), 0L, 255L));
    };
    return {channel(r), channel(g), channel(b), alpha};
}

Color Color::darker(double factor) const
{
    if (factor <= 0)
        return *this;
    if (factor < 1)
        return lighter(1 / factor);
    Hsv hsv = toHsv(*this);
    hsv.v /= factor;
    return fromHsv(hsv, a);
}

Color Color::lighter(double factor) const
{
    if (factor <= 0)
        return *this;
    if (factor < 1)
        return darker(1 / factor);
    Hsv hsv = toHsv(*this);
    hsv.v *= factor;
    // Past full value the colour turns towards white instead.
    if (hsv.v > 1) {
        hsv.s = std::max(0.0, hsv.s - (hsv.v - 1));
        hsv.v = 1;
    }
    return fromHsv(hsv, a);
}

} // namespace Lingmo::ui

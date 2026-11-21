// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/style.h"

#include "ui/glyph_run.h"
#include "ui/theme.h"

#include <cmath>
#include <memory>

namespace fm::style {

static const ui::FontMetrics& metrics()
{
    static uint64_t generation = 0;
    static std::unique_ptr<ui::FontMetrics> font;
    if (!font || generation != ui::Theme::generation()) {
        const ui::Theme& theme = ui::Theme::current();
        font = std::make_unique<ui::FontMetrics>(
                ui::FontSpec {theme.fontFamily.empty() ? std::string("sans-serif") : theme.fontFamily, theme.fontPointSize},
                1, false);
        generation = ui::Theme::generation();
    }
    return *font;
}

double fontHeight()
{
    // Unrounded.
    return metrics().heightF();
}

double advanceWidth(const std::string& text)
{
    return metrics().horizontalAdvanceF(text);
}

ui::Color selectedTextColor()
{
    const ui::Color color = ui::Theme::current().highlight;
    const double luminance = (color.r * 0.299 + color.g * 0.587 + color.b * 0.114) / 255.0;
    return luminance > 0.6 ? ui::Color::rgb(0x323238) : ui::Color::rgb(0xffffff);
}

ui::Color withAlpha(ui::Color color, double alpha)
{
    return color.withAlpha(alpha * color.a / 255.0);
}

ui::Color textAlpha(double alpha)
{
    return ui::Theme::current().text.withAlpha(alpha);
}

double bezier(double x1, double y1, double x2, double y2, double t)
{
    if (t <= 0)
        return 0;
    if (t >= 1)
        return 1;
    // Solve x(s) = t for s by Newton's method with a bisection fallback.
    const auto curve = [](double a, double b, double s) {
        const double inverse = 1 - s;
        return 3 * inverse * inverse * s * a + 3 * inverse * s * s * b + s * s * s;
    };
    double low = 0;
    double high = 1;
    double s = t;
    for (int i = 0; i < 30; ++i) {
        const double x = curve(x1, x2, s);
        if (std::abs(x - t) < 1e-7)
            break;
        if (x < t)
            low = s;
        else
            high = s;
        s = (low + high) / 2;
    }
    return curve(y1, y2, s);
}

} // namespace fm::style

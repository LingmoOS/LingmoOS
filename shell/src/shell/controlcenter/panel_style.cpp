// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/controlcenter/panel_style.h"

#include <cmath>

#include "shell/shell_data.h"
#include "core/preferences.h"
#include "base/settings.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include <string_view>

namespace Lingmo::Shell::Panel {

static ui::Color rgba(double red, double green, double blue, double alpha)
{
    return {uint8_t(std::lround(red * 255)), uint8_t(std::lround(green * 255)),
            uint8_t(std::lround(blue * 255)), uint8_t(std::lround(alpha * 255))};
}

bool dark()
{
    return ui::Theme::current().darkMode;
}

ui::Color tileColor()
{
    return dark() ? rgba(1, 1, 1, 0.1) : rgba(1, 1, 1, 0.8);
}

ui::Color tileInkColor()
{
    return ui::Theme::current().secondBackground;
}

ui::Color secondaryTextColor()
{
    return dark() ? rgba(1, 1, 1, 0.6) : ui::Theme::current().disabledText;
}

render::GlassParams panelGlass()
{
    const bool reduced = base::Settings(preferencesFile("services")).boolValue(
            "Appearance/ReduceTransparency", false);
    render::GlassParams glass = dark() ? popupGlass(0.13, 0.13, 0.14, 0.86)
                                       : popupGlass(0.94, 0.94, 0.95, 0.86);
    glass.blurSigma = glassFixed(20.0);
    glass.saturation = glassFixed(2.4);
    if (reduced) {
        // Keep the backdrop blur while removing refraction and increasing the
        // tint, so reduced transparency remains legible without becoming flat.
        glass.displacement = 0;
        glass.chroma = 0;
        glass.saturation = glassFixed(1.0);
        glass.tintAlpha = glassFixed(0.90);
    }
    return glass;
}

ui::Color statusSurfaceColor()
{
    return dark() ? ui::Color::rgb(0x242529) : ui::Color::rgb(0xFAFAFC);
}

ui::Color textColor(double opacity)
{
    const ui::Color text = ui::Theme::current().text;
    return text.withAlpha(text.a / 255.0 * opacity);
}

static double srgbToLinear(double value)
{
    return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
}

double relativeLuminance(double red, double green, double blue)
{
    return 0.2126 * srgbToLinear(red / 255.0) + 0.7152 * srgbToLinear(green / 255.0)
            + 0.0722 * srgbToLinear(blue / 255.0);
}

bool prefersWhite(double luminance)
{
    return 1.05 / (luminance + 0.05) >= 3.0;
}

bool isDark(ui::Color background)
{
    return prefersWhite(relativeLuminance(background.r, background.g, background.b));
}

ui::Color on(ui::Color background, double alpha)
{
    return isDark(background) ? rgba(1, 1, 1, alpha) : rgba(0, 0, 0, alpha);
}

ui::Color shade(ui::Color base, double amount)
{
    const double target = isDark(base) ? 1.0 : 0.0;
    // A 16-bit component and the 8 bits drawn round the same way.
    const auto towards = [amount, target](uint8_t channel) {
        const double value = channel / 255.0;
        return uint8_t(std::lround((value + (target - value) * amount) * 255));
    };
    return {towards(base.r), towards(base.g), towards(base.b), base.a};
}

ui::RectF parityPlate(const ui::RectF &slot, double glyph, double scale)
{
    const int slotPixels = int(std::lround(slot.width * scale));
    const int pixels = slotPixels - ((slotPixels - int(std::lround(glyph * scale))) % 2);
    const double size = pixels / scale;
    return {slot.x + (slot.width - size) / 2, slot.y + (slot.height - size) / 2, size, size};
}

void paintTile(ui::Painter &painter, const ui::RectF &rect)
{
    painter.fillRoundedRect(rect, kTileRadius, tileColor(), ui::Corners::Continuous);
}

std::string artwork(std::string_view name, bool white)
{
    return shellDataFile(std::string("controlcenter/") + (white ? "dark/" : "light/") + std::string(name)
                         + ".svg");
}

std::string artwork(std::string_view name)
{
    return shellDataFile(std::string("controlcenter/") + std::string(name) + ".svg");
}

} // namespace Lingmo::Shell::Panel

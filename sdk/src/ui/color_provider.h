// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "ui/color_id.h"
#include "ui/color_mixer.h"
#include "ui/geometry.h"
#include "ui/theme.h"

namespace Lingmo::ui {

// Every colour of a theme, made by the toolkit's mixer and then the
// application's, in the order they were added.
class ColorProvider
{
public:
    // The current theme's colours, until the theme or the mixers change.
    static const ColorProvider& current();
    // The current theme's colours in one colour mode, for a surface that
    // keeps a mode of its own, such as a dark header over a light window.
    static const ColorProvider& forMode(bool darkMode);
    // Adds an application's mixer after those added before; once, at start.
    static void addMixer(ColorMixerFunction mixer);

    explicit ColorProvider(const Theme& theme);

    ColorProvider(const ColorProvider&) = delete;
    ColorProvider& operator=(const ColorProvider&) = delete;

    template<ColorIdType Id>
    Color get(Id id) const
    {
        return get(uint16_t(id));
    }

    const Theme& theme() const { return m_theme; }
    bool darkMode() const { return m_theme.darkMode; }

private:
    friend class ColorRecipe;

    Color get(uint16_t id) const;

    Theme m_theme;
    std::vector<std::optional<ColorRecipe>> m_recipes;
    mutable std::vector<std::optional<Color>> m_colors;
    mutable std::vector<bool> m_resolving;
};

// The current theme's colour.
template<ColorIdType Id>
Color color(Id id)
{
    return ColorProvider::current().get(id);
}

} // namespace Lingmo::ui

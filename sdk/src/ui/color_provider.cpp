// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"

#include <cstdio>
#include <memory>
#include <utility>

#include "ui/toolkit_colors.h"

namespace Lingmo::ui {

// Shows a colour no mixer gave, or one made from itself.
static constexpr Color kPlaceholder = Color::rgb(0xFF00FF);

static std::vector<ColorMixerFunction>& mixers()
{
    static std::vector<ColorMixerFunction> functions;
    return functions;
}

static uint64_t s_mixerGeneration = 0;

namespace {

// The providers of one theme and set of mixers, made as they are asked for.
struct Providers
{
    uint64_t theme = UINT64_MAX;
    uint64_t mixers = UINT64_MAX;
    std::unique_ptr<ColorProvider> current;
    std::unique_ptr<ColorProvider> otherMode;
};

} // namespace

static Providers& providers()
{
    static Providers cached;
    if (cached.theme != Theme::generation() || cached.mixers != s_mixerGeneration) {
        cached.theme = Theme::generation();
        cached.mixers = s_mixerGeneration;
        cached.current.reset();
        cached.otherMode.reset();
    }
    return cached;
}

const ColorProvider& ColorProvider::current()
{
    Providers& cached = providers();
    if (!cached.current)
        cached.current = std::make_unique<ColorProvider>(Theme::current());
    return *cached.current;
}

const ColorProvider& ColorProvider::forMode(bool darkMode)
{
    const Theme& theme = Theme::current();
    if (darkMode == theme.darkMode)
        return current();
    Providers& cached = providers();
    if (!cached.otherMode) {
        // The other mode's colours with the appearance's accent.
        const Theme colors = darkMode ? Theme::dark() : Theme::light();
        Theme other = theme;
        other.darkMode = darkMode;
        other.background = colors.background;
        other.secondBackground = colors.secondBackground;
        other.alternateBackground = colors.alternateBackground;
        other.text = colors.text;
        other.disabledText = colors.disabledText;
        cached.otherMode = std::make_unique<ColorProvider>(other);
    }
    return *cached.otherMode;
}

void ColorProvider::addMixer(ColorMixerFunction mixer)
{
    mixers().push_back(std::move(mixer));
    ++s_mixerGeneration;
}

ColorProvider::ColorProvider(const Theme& theme)
    : m_theme(theme)
{
    const auto apply = [this](const ColorMixerFunction& function) {
        ColorMixer mixer;
        function(mixer, m_theme);
        for (auto& [id, recipe] : mixer.m_recipes) {
            if (id >= m_recipes.size())
                m_recipes.resize(size_t(id) + 1);
            m_recipes[id] = std::move(recipe);
        }
    };
    apply(addToolkitColors);
    for (const ColorMixerFunction& function : mixers())
        apply(function);
    m_colors.resize(m_recipes.size());
    m_resolving.resize(m_recipes.size());
}

Color ColorProvider::get(uint16_t id) const
{
    if (id >= m_recipes.size() || !m_recipes[id]) {
        fprintf(stderr, "lingmo-toolkit: no recipe for colour %u\n", unsigned(id));
        return kPlaceholder;
    }
    if (m_colors[id])
        return *m_colors[id];
    if (m_resolving[id]) {
        fprintf(stderr, "lingmo-toolkit: colour %u is made from itself\n", unsigned(id));
        return kPlaceholder;
    }
    m_resolving[id] = true;
    const Color color = m_recipes[id]->resolve(*this);
    m_resolving[id] = false;
    m_colors[id] = color;
    return color;
}

} // namespace Lingmo::ui

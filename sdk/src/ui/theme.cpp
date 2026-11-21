// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/theme.h"

#include "ui/application.h"
#include "ui/text_context.h"

namespace Lingmo::ui {

static Theme s_current = Theme::light();
static uint64_t s_generation = 0;

Theme Theme::light()
{
    Theme theme;
    theme.darkMode = false;
    theme.background = Color::rgb(0xF5F6FA);
    theme.secondBackground = Color::rgb(0xFFFFFF);
    theme.alternateBackground = Color::rgb(0xF2F4F5);
    theme.text = Color::rgb(0x323238);
    theme.disabledText = Color::rgb(0x64646E);
    theme.highlight = accent::blue;
    theme.highlightedText = Color::rgb(0xFFFFFF);
    return theme;
}

Theme Theme::dark()
{
    Theme theme;
    theme.darkMode = true;
    theme.background = Color::rgb(0x1C1C1D);
    theme.secondBackground = Color::rgb(0x2C2C2D);
    theme.alternateBackground = Color::rgb(0x3C3C3D);
    theme.text = Color::rgb(0xFFFFFF);
    theme.disabledText = Color::rgb(0x888888);
    theme.highlight = accent::blue;
    theme.highlightedText = Color::rgb(0xFFFFFF);
    return theme;
}

const Theme& Theme::current()
{
    return s_current;
}

void Theme::setCurrent(const Theme& theme)
{
    const bool fontChanged = theme.fontFamily != s_current.fontFamily;
    s_current = theme;
    if (Application* application = Application::instance(); application && fontChanged)
        application->fontsChanged();
    ++s_generation;
    if (Application* application = Application::instance())
        application->themeChanged();
}

uint64_t Theme::generation()
{
    return s_generation;
}

} // namespace Lingmo::ui

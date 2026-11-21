// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colors.h"
#include "syntaxtheme.h"

#include "ui/theme.h"

using namespace Lingmo::ui;

void addEditorColors(ColorMixer &mixer, const Theme &theme)
{
    const bool dark = theme.darkMode;

    mixer[EditorColor::FloatingCardShadow] = Color{0, 0, 0, 255}.withAlpha(dark ? 0.35 : 0.16);
    mixer[EditorColor::FloatingCardBorder] = withAlpha(ColorId::Text, dark ? 0.12 : 0.08);
    mixer[EditorColor::WindowBackground] = SyntaxTheme::windowBackground(dark);
}

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNTAXTHEME_H
#define SYNTAXTHEME_H

#include "syntax.h"

#include "ui/geometry.h"

// The text area's own colours and one per syntax style, in a light and a dark
// variant that follow the system appearance.
struct SyntaxTheme
{
    Lingmo::ui::Color background;
    Lingmo::ui::Color text;
    Lingmo::ui::Color gutterText;
    Lingmo::ui::Color currentLine;
    Lingmo::ui::Color selection;
    Lingmo::ui::Color match;
    Lingmo::ui::Color currentMatch;
    Lingmo::ui::Color bracket;
    Lingmo::ui::Color whitespace;
    Lingmo::ui::Color styles[int(SyntaxStyle::Count)];

    static SyntaxTheme theme(bool dark, Lingmo::ui::Color accent);
    // The background the theme gives the text area, which the window takes.
    static Lingmo::ui::Color windowBackground(bool dark);
};

#endif

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syntaxtheme.h"

using Lingmo::ui::Color;

struct Palette
{
    uint32_t background;
    uint32_t text;
    uint32_t comment;
    uint32_t keyword;
    uint32_t type;
    uint32_t constant;
    uint32_t string;
    uint32_t function;
    uint32_t variable;
    uint32_t preprocessor;
};

// One Light and One Dark, as their author defines them.
static constexpr Palette lightPalette{0xFFFFFF, 0x323238, 0xA0A1A7, 0xA626A4, 0x0184BC, 0x986801,
                                      0x50A14F, 0x4078F2, 0xE45649, 0xC18401};
static constexpr Palette darkPalette{0x232325, 0xE6E6E6, 0x7F848E, 0xC678DD, 0x56B6C2, 0xD19A66,
                                     0x98C379, 0x61AFEF, 0xE06C75, 0xE5C07B};

Color SyntaxTheme::windowBackground(bool dark)
{
    return Color::rgb((dark ? darkPalette : lightPalette).background);
}

SyntaxTheme SyntaxTheme::theme(bool dark, Color accent)
{
    const Palette &p = dark ? darkPalette : lightPalette;
    SyntaxTheme t;
    t.background = Color::rgb(p.background);
    t.text = Color::rgb(p.text);
    t.gutterText = t.text.withAlpha(dark ? 0.35 : 0.4);
    t.currentLine = t.text.withAlpha(dark ? 0.05 : 0.04);
    t.selection = accent.withAlpha(dark ? 0.4 : 0.25);
    t.match = dark ? Color{255, 196, 0, 70} : Color{255, 200, 0, 90};
    t.currentMatch = dark ? Color{255, 152, 0, 150} : Color{255, 152, 0, 140};
    t.bracket = accent.withAlpha(dark ? 0.35 : 0.22);
    t.whitespace = t.text.withAlpha(0.3);

    const Color comment = Color::rgb(p.comment);
    const Color keyword = Color::rgb(p.keyword);
    const Color type = Color::rgb(p.type);
    const Color constant = Color::rgb(p.constant);
    const Color string = Color::rgb(p.string);
    const Color function = Color::rgb(p.function);
    const Color variable = Color::rgb(p.variable);
    const Color preprocessor = Color::rgb(p.preprocessor);

    auto set = [&t](SyntaxStyle style, Color color) { t.styles[int(style)] = color; };
    set(SyntaxStyle::Normal, t.text);
    set(SyntaxStyle::Keyword, keyword);
    set(SyntaxStyle::Type, type);
    set(SyntaxStyle::Constant, constant);
    set(SyntaxStyle::String, string);
    set(SyntaxStyle::Escape, type);
    set(SyntaxStyle::Number, constant);
    set(SyntaxStyle::Comment, comment);
    set(SyntaxStyle::Preprocessor, preprocessor);
    set(SyntaxStyle::Function, function);
    set(SyntaxStyle::Variable, variable);
    set(SyntaxStyle::Heading, function);
    set(SyntaxStyle::Emphasis, t.text);
    set(SyntaxStyle::Strong, t.text);
    set(SyntaxStyle::Code, string);
    set(SyntaxStyle::Link, function);
    set(SyntaxStyle::Tag, variable);
    set(SyntaxStyle::Attribute, constant);
    set(SyntaxStyle::Key, function);
    set(SyntaxStyle::Section, keyword);
    set(SyntaxStyle::Added, string);
    set(SyntaxStyle::Removed, variable);
    set(SyntaxStyle::DiffHeader, comment);
    set(SyntaxStyle::Error, variable);
    set(SyntaxStyle::Warning, constant);
    return t;
}

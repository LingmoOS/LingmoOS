/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "quicktheme.h"

namespace Lingmo
{
QuickTheme::QuickTheme(QObject *parent)
    : Theme(parent)
{
    connect(this, &Theme::themeChanged, this, &QuickTheme::themeChangedProxy);
}

QuickTheme::~QuickTheme()
{
}

QColor QuickTheme::textColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor);
}

QColor QuickTheme::highlightColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HighlightColor);
}

QColor QuickTheme::highlightedTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HighlightedTextColor);
}

QColor QuickTheme::positiveTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::PositiveTextColor);
}

QColor QuickTheme::neutralTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::NeutralTextColor);
}

QColor QuickTheme::negativeTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::NegativeTextColor);
}

QColor QuickTheme::disabledTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::DisabledTextColor);
}

QColor QuickTheme::backgroundColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::BackgroundColor);
}

QColor QuickTheme::buttonTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonBackgroundColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonPositiveTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonNeutralTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonNegativeTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::linkColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::LinkColor);
}

QColor QuickTheme::visitedLinkColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::VisitedLinkColor);
}

QColor QuickTheme::buttonHoverColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HoverColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonFocusColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::FocusColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::buttonHighlightedTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::ButtonColorGroup);
}

QColor QuickTheme::viewTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::viewBackgroundColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::viewHoverColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HoverColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::viewFocusColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::FocusColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::viewHighlightedTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::viewPositiveTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::viewNeutralTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::viewNegativeTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ViewColorGroup);
}

QColor QuickTheme::complementaryTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryBackgroundColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryHoverColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HoverColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryFocusColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::FocusColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryHighlightedTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryPositiveTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryNeutralTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::complementaryNegativeTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::ComplementaryColorGroup);
}

QColor QuickTheme::headerTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerBackgroundColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerHoverColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HoverColor, Lingmo::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerFocusColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::FocusColor, Lingmo::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerHighlightedTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerPositiveTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerNeutralTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::HeaderColorGroup);
}

QColor QuickTheme::headerNegativeTextColor() const
{
    return Lingmo::Theme::color(Lingmo::Theme::TextColor, Lingmo::Theme::HeaderColorGroup);
}
}

#include "moc_quicktheme.cpp"

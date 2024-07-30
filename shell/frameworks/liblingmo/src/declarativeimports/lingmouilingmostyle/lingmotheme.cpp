/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lingmotheme.h"
#include <KIconLoader>
#include <QDebug>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScopeGuard>

#include <KColorScheme>
#include <KConfigGroup>
#include <KIconColors>

LingmoTheme::LingmoTheme(QObject *parent)
    : PlatformTheme(parent)
{
    setSupportsIconColoring(true);

    auto parentItem = qobject_cast<QQuickItem *>(parent);
    if (parentItem) {
        connect(parentItem, &QQuickItem::enabledChanged, this, &LingmoTheme::syncColors);
        connect(parentItem, &QQuickItem::visibleChanged, this, [this, parentItem] {
            if (!parentItem->isVisible()) {
                return;
            }
            syncColors();
        });
    }

    setDefaultFont(qGuiApp->font());

    KSharedConfigPtr ptr = KSharedConfig::openConfig();
    KConfigGroup general(ptr->group(QStringLiteral("general")));

    setSmallFont(general.readEntry("smallestReadableFont", []() {
        auto smallFont = qApp->font();
#ifndef Q_OS_WIN
        if (smallFont.pixelSize() != -1) {
            smallFont.setPixelSize(smallFont.pixelSize() - 2);
        } else {
            smallFont.setPointSize(smallFont.pointSize() - 2);
        }
#endif
        return smallFont;
    }()));

    syncColors();
    connect(&m_theme, &Lingmo::Theme::themeChanged, this, &LingmoTheme::syncColors);
}

LingmoTheme::~LingmoTheme()
{
}

QIcon LingmoTheme::iconFromTheme(const QString &name, const QColor &customColor)
{
    KIconColors colors(Lingmo::Theme::globalPalette());
    KColorScheme colorScheme(QPalette::Active, KColorScheme::Window, Lingmo::Theme::globalColorScheme());

    colors.setPositiveText(colorScheme.foreground(KColorScheme::PositiveText).color().name());
    colors.setNeutralText(colorScheme.foreground(KColorScheme::NeutralText).color().name());
    colors.setNegativeText(colorScheme.foreground(KColorScheme::NegativeText).color().name());
    colors.setActiveText(colorScheme.foreground(KColorScheme::ActiveText).color().name());

    if (customColor != Qt::transparent) {
        colors.setText(customColor);
    }

    return KDE::icon(name, colors);
}

void LingmoTheme::syncColors()
{
    if (QCoreApplication::closingDown()) {
        return;
    }

    Lingmo::Theme::ColorGroup group;
    switch (colorSet()) {
    case View:
        group = Lingmo::Theme::ViewColorGroup;
        break;
    case Button:
        group = Lingmo::Theme::ButtonColorGroup;
        break;
    case Tooltip:
        group = Lingmo::Theme::ToolTipColorGroup;
        break;
    case Complementary:
        group = Lingmo::Theme::ComplementaryColorGroup;
        break;
    case Header:
        group = Lingmo::Theme::HeaderColorGroup;
        break;
    case Selection: // Lingmo::Theme doesn't have selection group
    case Window:
    default:
        group = Lingmo::Theme::NormalColorGroup;
    }

    // foreground
    setTextColor(m_theme.color(Lingmo::Theme::TextColor, group));
    setDisabledTextColor(m_theme.color(Lingmo::Theme::DisabledTextColor, group));
    setHighlightedTextColor(m_theme.color(Lingmo::Theme::HighlightedTextColor, group));
    // Lingmo::Theme doesn't have ActiveText, use PositiveTextColor
    setActiveTextColor(m_theme.color(Lingmo::Theme::PositiveTextColor, group));
    setLinkColor(m_theme.color(Lingmo::Theme::LinkColor, group));
    setVisitedLinkColor(m_theme.color(Lingmo::Theme::VisitedLinkColor, group));
    setNegativeTextColor(m_theme.color(Lingmo::Theme::NegativeTextColor, group));
    setNeutralTextColor(m_theme.color(Lingmo::Theme::NeutralTextColor, group));
    setPositiveTextColor(m_theme.color(Lingmo::Theme::PositiveTextColor, group));

    // background
    setBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));
    setHighlightColor(m_theme.color(Lingmo::Theme::HighlightColor, group));
    // Lingmo::Theme doesn't have AlternateBackground
    setAlternateBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));

    // Lingmo::Theme doesn't have any different background color type
    setActiveBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));
    setLinkBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));
    setVisitedLinkBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));
    setNegativeBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));
    setNeutralBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));
    setPositiveBackgroundColor(m_theme.color(Lingmo::Theme::BackgroundColor, group));

    // decoration
    setHoverColor(m_theme.color(Lingmo::Theme::HoverColor, group));
    setFocusColor(m_theme.color(Lingmo::Theme::FocusColor, group));
}

bool LingmoTheme::event(QEvent *event)
{
    if (event->type() == LingmoUI::Platform::PlatformThemeEvents::ColorSetChangedEvent::type) {
        syncColors();
    }

    if (event->type() == LingmoUI::Platform::PlatformThemeEvents::ColorGroupChangedEvent::type) {
        syncColors();
    }

    return PlatformTheme::event(event);
}

#include "moc_lingmotheme.cpp"

/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "coloreditor.h"

#include <QDebug>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>

ColorEditor::ColorEditor(QObject *parent)
    : QObject(parent)
{
}

ColorEditor::~ColorEditor()
{
}

void ColorEditor::load()
{
    const QString colorsFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/desktoptheme/" + m_theme + "/colors";

    KConfig c(colorsFile);

    KConfigGroup cg(&c, "Colors:Window");

    m_textColor = cg.readEntry("ForegroundNormal", QColor());
    m_highlightColor = cg.readEntry("DecorationHover", QColor());
    m_backgroundColor = cg.readEntry("BackgroundNormal", QColor());
    m_linkColor = cg.readEntry("ForegroundLink", QColor());
    m_visitedLinkColor = cg.readEntry("ForegroundVisited", QColor());

    cg = KConfigGroup(&c, "Colors:Button");
    m_buttonTextColor = cg.readEntry("ForegroundNormal", QColor());
    m_buttonBackgroundColor = cg.readEntry("BackgroundNormal", QColor());
    m_buttonHoverColor = cg.readEntry("DecorationHover", QColor());
    m_buttonFocusColor = cg.readEntry("DecorationFocus", QColor());

    cg = KConfigGroup(&c, "Colors:View");
    m_viewTextColor = cg.readEntry("ForegroundNormal", QColor());
    m_viewBackgroundColor = cg.readEntry("BackgroundNormal", QColor());
    m_viewHoverColor = cg.readEntry("DecorationHover", QColor());
    m_viewFocusColor = cg.readEntry("DecorationFocus", QColor());

    cg = KConfigGroup(&c, "Colors:Complementary");
    m_complementaryTextColor = cg.readEntry("ForegroundNormal", QColor());
    m_complementaryBackgroundColor = cg.readEntry("BackgroundNormal", QColor());
    m_complementaryHoverColor = cg.readEntry("DecorationHover", QColor());
    m_complementaryFocusColor = cg.readEntry("DecorationFocus", QColor());

    emit colorsChanged();
}

void ColorEditor::save()
{
    const QString colorsFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/desktoptheme/" + m_theme + "/colors";

    KConfig c(colorsFile);

    KConfigGroup cg(&c, "Colors:Window");

    cg.writeEntry("ForegroundNormal", m_textColor);
    cg.writeEntry("DecorationHover", m_highlightColor);
    cg.writeEntry("BackgroundNormal", m_backgroundColor);
    cg.writeEntry("ForegroundLink", m_linkColor);
    cg.writeEntry("ForegroundVisited", m_visitedLinkColor);
    cg.sync();

    cg = KConfigGroup(&c, "Colors:Button");
    cg.writeEntry("ForegroundNormal", m_buttonTextColor);
    cg.writeEntry("BackgroundNormal", m_buttonBackgroundColor);
    cg.writeEntry("DecorationHover", m_buttonHoverColor);
    cg.writeEntry("DecorationFocus", m_buttonFocusColor);
    cg.sync();

    cg = KConfigGroup(&c, "Colors:View");
    cg.writeEntry("ForegroundNormal", m_viewTextColor);
    cg.writeEntry("BackgroundNormal", m_viewBackgroundColor);
    cg.writeEntry("DecorationHover", m_viewHoverColor);
    cg.writeEntry("DecorationFocus", m_viewFocusColor);
    cg.sync();

    cg = KConfigGroup(&c, "Colors:Complementary");
    cg.writeEntry("ForegroundNormal", m_complementaryTextColor);
    cg.writeEntry("BackgroundNormal", m_complementaryBackgroundColor);
    cg.writeEntry("DecorationHover", m_complementaryHoverColor);
    cg.writeEntry("DecorationFocus", m_complementaryFocusColor);
    cg.sync();
}

QString ColorEditor::theme() const
{
    return m_theme;
}

void ColorEditor::setTheme(const QString &theme)
{
    if (theme == m_theme) {
        return;
    }

    m_theme = theme;
    emit themeChanged();
    load();
}

QColor ColorEditor::textColor() const
{
    return m_textColor;
}

void ColorEditor::setTextColor(const QColor &color)
{
    if (color == m_textColor) {
        return;
    }

    m_textColor = color;
    emit colorsChanged();
}

QColor ColorEditor::highlightColor() const
{
    return m_highlightColor;
}

void ColorEditor::setHighlightColor(const QColor &color)
{
    if (color == m_highlightColor) {
        return;
    }

    m_highlightColor = color;
    emit colorsChanged();
}

QColor ColorEditor::backgroundColor() const
{
    return m_backgroundColor;
}

void ColorEditor::setBackgroundColor(const QColor &color)
{
    if (color == m_backgroundColor) {
        return;
    }

    m_backgroundColor = color;
    emit colorsChanged();
}

QColor ColorEditor::buttonTextColor() const
{
    return m_buttonTextColor;
}

void ColorEditor::setButtonTextColor(const QColor &color)
{
    if (color == m_buttonTextColor) {
        return;
    }

    m_buttonTextColor = color;
    emit colorsChanged();
}

QColor ColorEditor::buttonBackgroundColor() const
{
    return m_buttonBackgroundColor;
}

void ColorEditor::setButtonBackgroundColor(const QColor &color)
{
    if (color == m_buttonBackgroundColor) {
        return;
    }

    m_buttonBackgroundColor = color;
    emit colorsChanged();
}

QColor ColorEditor::linkColor() const
{
    return m_linkColor;
}

void ColorEditor::setLinkColor(const QColor &color)
{
    if (color == m_linkColor) {
        return;
    }

    m_linkColor = color;
    emit colorsChanged();
}

QColor ColorEditor::visitedLinkColor() const
{
    return m_visitedLinkColor;
}

void ColorEditor::setVisitedLinkColor(const QColor &color)
{
    if (color == m_visitedLinkColor) {
        return;
    }

    m_visitedLinkColor = color;
    emit colorsChanged();
}

QColor ColorEditor::buttonHoverColor() const
{
    return m_buttonHoverColor;
}

void ColorEditor::setButtonHoverColor(const QColor &color)
{
    if (color == m_buttonHoverColor) {
        return;
    }

    m_buttonHoverColor = color;
    emit colorsChanged();
}

QColor ColorEditor::buttonFocusColor() const
{
    return m_buttonFocusColor;
}

void ColorEditor::setButtonFocusColor(const QColor &color)
{
    if (color == m_buttonFocusColor) {
        return;
    }

    m_buttonFocusColor = color;
    emit colorsChanged();
}

QColor ColorEditor::viewTextColor() const
{
    return m_viewTextColor;
}

void ColorEditor::setViewTextColor(const QColor &color)
{
    if (color == m_viewTextColor) {
        return;
    }

    m_viewTextColor = color;
    emit colorsChanged();
}

QColor ColorEditor::viewBackgroundColor() const
{
    return m_viewBackgroundColor;
}

void ColorEditor::setViewBackgroundColor(const QColor &color)
{
    if (color == m_viewBackgroundColor) {
        return;
    }

    m_viewBackgroundColor = color;
    emit colorsChanged();
}

QColor ColorEditor::viewHoverColor() const
{
    return m_viewHoverColor;
}

void ColorEditor::setViewHoverColor(const QColor &color)
{
    if (color == m_viewHoverColor) {
        return;
    }

    m_viewHoverColor = color;
    emit colorsChanged();
}

QColor ColorEditor::viewFocusColor() const
{
    return m_viewFocusColor;
}

void ColorEditor::setViewFocusColor(const QColor &color)
{
    if (color == m_viewFocusColor) {
        return;
    }

    m_viewFocusColor = color;
    emit colorsChanged();
}

QColor ColorEditor::complementaryTextColor() const
{
    return m_complementaryTextColor;
}

void ColorEditor::setComplementaryTextColor(const QColor &color)
{
    if (color == m_complementaryTextColor) {
        return;
    }

    m_complementaryTextColor = color;
    emit colorsChanged();
}

QColor ColorEditor::complementaryBackgroundColor() const
{
    return m_complementaryBackgroundColor;
}

void ColorEditor::setComplementaryBackgroundColor(const QColor &color)
{
    if (color == m_complementaryBackgroundColor) {
        return;
    }

    m_complementaryBackgroundColor = color;
    emit colorsChanged();
}

QColor ColorEditor::complementaryHoverColor() const
{
    return m_complementaryHoverColor;
}

void ColorEditor::setComplementaryHoverColor(const QColor &color)
{
    if (color == m_complementaryHoverColor) {
        return;
    }

    m_complementaryHoverColor = color;
    emit colorsChanged();
}

QColor ColorEditor::complementaryFocusColor() const
{
    return m_complementaryFocusColor;
}

void ColorEditor::setComplementaryFocusColor(const QColor &color)
{
    if (color == m_complementaryHoverColor) {
        return;
    }

    m_complementaryFocusColor = color;
    emit colorsChanged();
}

#include "moc_coloreditor.cpp"

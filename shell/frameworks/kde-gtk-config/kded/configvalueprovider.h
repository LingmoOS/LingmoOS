/*
 * SPDX-FileCopyrightText: 2019 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QTemporaryDir>

#include <KSharedConfig>

class QString;
class QFont;

class ConfigValueProvider
{
public:
    ConfigValueProvider();

    QString fontName() const;
    QString iconThemeName() const;
    QString cursorThemeName() const;
    QString soundThemeName() const;
    bool eventSoundsEnabled() const;
    int cursorSize() const;
    bool iconsOnButtons() const;
    bool iconsInMenus() const;
    int toolbarStyle() const;
    bool scrollbarBehavior() const;
    bool preferDarkTheme() const;
    QString windowDecorationsButtonsOrder() const;
    QStringList windowDecorationsButtonsImages() const;
    bool enableAnimations() const;
    int doubleClickInterval() const;
    QMap<QString, QColor> colors() const;

    // The global scale factor, as X11 doesn't support mixed-DPI
    // setups. It's useful both for Plasma/X11 sessions and for
    // XWayland apps in Plasma/Wayland sessions.
    //
    // On Wayland sessions returns 1.0 if XWayland client scaling
    // is disabled.
    double x11GlobalScaleFactor() const;

    // The Force Font DPI setting from the fonts kcm. Returns 0
    // if disabled
    int fontDpi() const;

private:
    QString fontStyleHelper(const QFont &font) const;
    QString windowDecorationButtonsOrderInGtkNotation(const QString &kdeConfigValue) const;
    QString currentWindowDecorationPluginPath() const;

    KSharedConfigPtr kdeglobalsConfig;
    KSharedConfigPtr fontsConfig;
    KSharedConfigPtr inputConfig;
    KSharedConfigPtr kwinConfig;

    QTemporaryDir generatedCSDTempPath;
};

/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 Benjamin Port <benjamin.port@ben2367.fr>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindowconfig.h"
#include "ksharedconfig.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

static const char s_initialSizePropertyName[] = "_kconfig_initial_size";
static const char s_initialScreenSizePropertyName[] = "_kconfig_initial_screen_size";

// Convenience function to get a space-separated list of all connected screens
static QString allConnectedScreens()
{
    QStringList names;
    const auto screens = QGuiApplication::screens();
    names.reserve(screens.length());
    for (auto screen : screens) {
        names << screen->name();
    }
    // A string including the connector names is used in the config file key for
    // storing per-screen-arrangement size and position data, which means we
    // need this string to be consistent for the same screen arrangement. But
    // connector order is non-deterministic. We need to sort the list to keep a
    // consistent order and avoid losing multi-screen size and position data.
    names.sort();
    return names.join(QLatin1Char(' '));
}

// Convenience function to return screen by its name from window screen siblings
// returns current window screen if not found
static QScreen *findScreenByName(const QWindow *window, const QString screenName)
{
    if (screenName == window->screen()->name()) {
        return window->screen();
    }
    for (QScreen *s : window->screen()->virtualSiblings()) {
        if (s->name() == screenName) {
            return s;
        }
    }
    return window->screen();
}

// Convenience function to get an appropriate config file key under which to
// save window size, position, or maximization information.
static QString configFileString(const QString &key)
{
    QString returnString;
    const int numberOfScreens = QGuiApplication::screens().length();

    if (numberOfScreens == 1) {
        // For single-screen setups, we save data on a per-resolution basis.
        const QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        returnString = QStringLiteral("%1x%2 screen: %3").arg(QString::number(screenGeometry.width()), QString::number(screenGeometry.height()), key);
    } else {
        // For multi-screen setups, we save data based on the number of screens.
        // Distinguishing individual screens based on their names is unreliable
        // due to name strings being inherently volatile.
        returnString = QStringLiteral("%1 screens: %2").arg(QString::number(numberOfScreens), key);
    }
    return returnString;
}

// Convenience function for "window is maximized" string
static QString screenMaximizedString()
{
    return configFileString(QStringLiteral("Window-Maximized"));
}
// Convenience function for window width string
static QString windowWidthString()
{
    return configFileString(QStringLiteral("Width"));
}
// Convenience function for window height string
static QString windowHeightString()
{
    return configFileString(QStringLiteral("Height"));
}
// Convenience function for window X position string
static QString windowXPositionString()
{
    return configFileString(QStringLiteral("XPosition"));
}
// Convenience function for window Y position string
static QString windowYPositionString()
{
    return configFileString(QStringLiteral("YPosition"));
}
static QString windowScreenPositionString()
{
    return QStringLiteral("%1").arg(allConnectedScreens());
}

void KWindowConfig::saveWindowSize(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options)
{
    // QWindow::screen() shouldn't return null, but it sometimes does due to bugs.
    if (!window || !window->screen()) {
        return;
    }
    const QScreen *screen = window->screen();

    const QSize sizeToSave = window->size();
    const bool isMaximized = window->windowState() & Qt::WindowMaximized;

    // Save size only if window is not maximized
    if (!isMaximized) {
        const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
        const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
        const bool sizeValid = defaultSize.isValid() && defaultScreenSize.isValid();
        if (!sizeValid || (sizeValid && (defaultSize != sizeToSave || defaultScreenSize != screen->geometry().size()))) {
            config.writeEntry(windowWidthString(), sizeToSave.width(), options);
            config.writeEntry(windowHeightString(), sizeToSave.height(), options);
            // Don't keep the maximized string in the file since the window is
            // no longer maximized at this point
            config.deleteEntry(screenMaximizedString());
        }
    }
    if ((isMaximized == false) && !config.hasDefault(screenMaximizedString())) {
        config.revertToDefault(screenMaximizedString());
    } else {
        config.writeEntry(screenMaximizedString(), isMaximized, options);
    }
}

bool KWindowConfig::hasSavedWindowSize(KConfigGroup &config)
{
    return config.hasKey(windowWidthString()) || config.hasKey(windowHeightString()) || config.hasKey(screenMaximizedString());
}

void KWindowConfig::restoreWindowSize(QWindow *window, const KConfigGroup &config)
{
    if (!window) {
        return;
    }

    const QString screenName = config.readEntry(windowScreenPositionString(), window->screen()->name());

    const int width = config.readEntry(windowWidthString(), -1);
    const int height = config.readEntry(windowHeightString(), -1);
    const bool isMaximized = config.readEntry(configFileString(QStringLiteral("Window-Maximized")), false);

    // Check default size
    const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
    const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
    if (!defaultSize.isValid() || !defaultScreenSize.isValid()) {
        const QScreen *screen = findScreenByName(window, screenName);
        window->setProperty(s_initialSizePropertyName, window->size());
        window->setProperty(s_initialScreenSizePropertyName, screen->geometry().size());
    }

    if (width > 0 && height > 0) {
        window->resize(width, height);
    }

    if (isMaximized) {
        window->setWindowState(Qt::WindowMaximized);
    }
}

void KWindowConfig::saveWindowPosition(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options)
{
    // On Wayland, the compositor is solely responsible for window positioning,
    // So this needs to be a no-op
    if (!window || QGuiApplication::platformName() == QLatin1String{"wayland"}) {
        return;
    }

    // If the window is maximized, saving the position will only serve to mis-position
    // it once de-maximized, so let's not do that
    if (window->windowState() & Qt::WindowMaximized) {
        return;
    }

    config.writeEntry(windowXPositionString(), window->x(), options);
    config.writeEntry(windowYPositionString(), window->y(), options);
    config.writeEntry(windowScreenPositionString(), window->screen()->name(), options);
}

bool KWindowConfig::hasSavedWindowPosition(KConfigGroup &config)
{
    // Window position save/restore features outside of the compositor are not
    // supported on Wayland
    if (QGuiApplication::platformName() == QLatin1String{"wayland"}) {
        return false;
    }

    return config.hasKey(windowXPositionString()) || config.hasKey(windowYPositionString()) || config.hasKey(windowScreenPositionString());
}

void KWindowConfig::restoreWindowPosition(QWindow *window, const KConfigGroup &config)
{
    // On Wayland, the compositor is solely responsible for window positioning,
    // So this needs to be a no-op
    if (!window || QGuiApplication::platformName() == QLatin1String{"wayland"}) {
        return;
    }

    const bool isMaximized = config.readEntry(configFileString(QStringLiteral("Window-Maximized")), false);

    // Don't need to restore position if the window was maximized
    if (isMaximized) {
        window->setWindowState(Qt::WindowMaximized);
        return;
    }

    // Move window to proper screen
    const QScreen *screen = window->screen();
    const QString screenName = config.readEntry(windowScreenPositionString(), screen->name());
    if (screenName != screen->name()) {
        QScreen *screenConf = findScreenByName(window, screenName);
        window->setScreen(screenConf);
        restoreWindowScreenPosition(window, screenConf, config);
        return;
    }
    restoreWindowScreenPosition(window, screen, config);
}

void KWindowConfig::restoreWindowScreenPosition(QWindow *window, const QScreen *screen, const KConfigGroup &config)
{
    Q_UNUSED(screen);
    const int xPos = config.readEntry(windowXPositionString(), -1);
    const int yPos = config.readEntry(windowYPositionString(), -1);

    if (xPos == -1 || yPos == -1) {
        return;
    }

    window->setX(xPos);
    window->setY(yPos);
}

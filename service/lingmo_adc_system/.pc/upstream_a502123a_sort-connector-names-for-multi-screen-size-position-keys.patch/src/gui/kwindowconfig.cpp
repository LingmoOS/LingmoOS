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

// QScreen::name() returns garbage on Windows; see https://bugreports.qt.io/browse/QTBUG-74317
// So we use the screens' serial numbers to identify them instead
// FIXME: remove this once we can depend on Qt 6.4, where this is fixed
#if defined(Q_OS_WINDOWS) && QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#define SCREENNAME serialNumber
#else
#define SCREENNAME name
#endif

static const char s_initialSizePropertyName[] = "_kconfig_initial_size";
static const char s_initialScreenSizePropertyName[] = "_kconfig_initial_screen_size";

// Convenience function to get a space-separated list of all connected screens
static QString allConnectedScreens()
{
    QStringList names;
    const auto screens = QGuiApplication::screens();
    names.reserve(screens.length());
    for (auto screen : screens) {
        names << screen->SCREENNAME();
    }
    return names.join(QLatin1Char(' '));
}

// Convenience function to return screen by its name from window screen siblings
// returns current window screen if not found
static QScreen *findScreenByName(const QWindow *window, const QString screenName)
{
    if (screenName == window->screen()->SCREENNAME()) {
        return window->screen();
    }
    for (QScreen *s : window->screen()->virtualSiblings()) {
        if (s->SCREENNAME() == screenName) {
            return s;
        }
    }
    return window->screen();
}

// Convenience function to get an appropriate config file key under which to
// save window size, position, or maximization information.
static QString configFileString(const QScreen *screen, const QString &key)
{
    // We include resolution data to also save data on a per-resolution basis
    const QString returnString =
        QStringLiteral("%1 %2 %3x%4 %5")
            .arg(allConnectedScreens(), key, QString::number(screen->geometry().width()), QString::number(screen->geometry().height()), screen->SCREENNAME());
    return returnString;
}

// Convenience function for "window is maximized" string
static QString screenMaximizedString(const QScreen *screen)
{
    return configFileString(screen, QStringLiteral("Window-Maximized"));
}
// Convenience function for window width string
static QString windowWidthString(const QScreen *screen)
{
    return configFileString(screen, QStringLiteral("Width"));
}
// Convenience function for window height string
static QString windowHeightString(const QScreen *screen)
{
    return configFileString(screen, QStringLiteral("Height"));
}
// Convenience function for window X position string
static QString windowXPositionString(const QScreen *screen)
{
    return configFileString(screen, QStringLiteral("XPosition"));
}
// Convenience function for window Y position string
static QString windowYPositionString(const QScreen *screen)
{
    return configFileString(screen, QStringLiteral("YPosition"));
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
            config.writeEntry(windowWidthString(screen), sizeToSave.width(), options);
            config.writeEntry(windowHeightString(screen), sizeToSave.height(), options);
            // Don't keep the maximized string in the file since the window is
            // no longer maximized at this point
            config.deleteEntry(screenMaximizedString(screen));
        }
    }
    if ((isMaximized == false) && !config.hasDefault(screenMaximizedString(screen))) {
        config.revertToDefault(screenMaximizedString(screen));
    } else {
        config.writeEntry(screenMaximizedString(screen), isMaximized, options);
    }
}

void KWindowConfig::restoreWindowSize(QWindow *window, const KConfigGroup &config)
{
    if (!window) {
        return;
    }

    const QString screenName = config.readEntry(windowScreenPositionString(), window->screen()->SCREENNAME());
    const QScreen *screen = findScreenByName(window, screenName);

    // Fall back to non-per-screen-arrangement info if it's available but
    // per-screen-arrangement information is not
    // TODO: Remove in KF6 or maybe even KF5.80 or something. It really only needs
    // to be here to transition existing users once they upgrade from 5.73 -> 5.74
    const int fallbackWidth = config.readEntry(QStringLiteral("Width %1").arg(screen->geometry().width()), window->size().width());
    const int fallbackHeight = config.readEntry(QStringLiteral("Height %1").arg(screen->geometry().height()), window->size().height());

    const int width = config.readEntry(windowWidthString(screen), fallbackWidth);
    const int height = config.readEntry(windowHeightString(screen), fallbackHeight);
    const bool isMaximized = config.readEntry(configFileString(screen, QStringLiteral("Window-Maximized")), false);

    // Check default size
    const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
    const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
    if (!defaultSize.isValid() || !defaultScreenSize.isValid()) {
        window->setProperty(s_initialSizePropertyName, window->size());
        window->setProperty(s_initialScreenSizePropertyName, screen->geometry().size());
    }

    // If window is maximized set maximized state and in all case set the size
    window->resize(width, height);
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

    const QScreen *screen = window->screen();
    config.writeEntry(windowXPositionString(screen), window->x(), options);
    config.writeEntry(windowYPositionString(screen), window->y(), options);
    config.writeEntry(windowScreenPositionString(), screen->SCREENNAME(), options);
}

void KWindowConfig::restoreWindowPosition(QWindow *window, const KConfigGroup &config)
{
    // On Wayland, the compositor is solely responsible for window positioning,
    // So this needs to be a no-op
    if (!window || QGuiApplication::platformName() == QLatin1String{"wayland"}) {
        return;
    }

    const QScreen *screen = window->screen();
    const bool isMaximized = config.readEntry(configFileString(screen, QStringLiteral("Window-Maximized")), false);

    // Don't need to restore position if the window was maximized
    if (isMaximized) {
        window->setWindowState(Qt::WindowMaximized);
        return;
    }

    // Move window to proper screen
    const QString screenName = config.readEntry(windowScreenPositionString(), screen->SCREENNAME());
    if (screenName != screen->SCREENNAME()) {
        QScreen *screenConf = findScreenByName(window, screenName);
        window->setScreen(screenConf);
        restoreWindowScreenPosition(window, screenConf, config);
        return;
    }
    restoreWindowScreenPosition(window, screen, config);
}

void KWindowConfig::restoreWindowScreenPosition(QWindow *window, const QScreen *screen, const KConfigGroup &config)
{
    const QRect desk = window->screen()->geometry();
    // Fall back to non-per-resolution info if it's available but
    // per-resolution information is not
    // TODO: Remove in KF6 or maybe even KF5.85 or something. It really only needs
    // to be here to transition existing users once they upgrade from 5.78 -> 5.79
    const int fallbackXPosition = config.readEntry(QStringLiteral("%1 XPosition %2").arg(allConnectedScreens(), QString::number(desk.width())), -1);
    const int fallbackYPosition = config.readEntry(QStringLiteral("%1 YPosition %2").arg(allConnectedScreens(), QString::number(desk.height())), -1);
    const int xPos = config.readEntry(windowXPositionString(screen), fallbackXPosition);
    const int yPos = config.readEntry(windowYPositionString(screen), fallbackYPosition);

    if (xPos == -1 || yPos == -1) {
        return;
    }

    window->setX(xPos);
    window->setY(yPos);
}

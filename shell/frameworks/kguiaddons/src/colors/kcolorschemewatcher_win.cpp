/*
 * SPDX-FileCopyrightText: 2020 Piyush Aggarwal <piyushaggarwal002@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcolorschemewatcher_win.h"

#include <windows.h>

#include <QAbstractEventDispatcher>

#include <QDebug>

KColorSchemeWatcherWin::KColorSchemeWatcherWin()
{
    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

    m_preferDarkMode = !(m_settings.value(QStringLiteral("AppsUseLightTheme"), true).value<bool>());
}

bool KColorSchemeWatcherWin::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    MSG *msg = static_cast<MSG *>(message);
    switch (msg->message) {
    case WM_SETTINGCHANGE: {
        m_settings.sync();
        const bool preferDarkModeNow = !(m_settings.value(QStringLiteral("AppsUseLightTheme"), true).value<bool>());
        if (m_preferDarkMode != preferDarkModeNow) {
            m_preferDarkMode = preferDarkModeNow;
            Q_EMIT systemPreferenceChanged();
        }

        break;
    }
    default: {
    }
    }
    return false;
}

KColorSchemeWatcher::ColorPreference KColorSchemeWatcherWin::systemPreference() const
{
    return m_preferDarkMode ? KColorSchemeWatcher::PreferDark : KColorSchemeWatcher::PreferLight;
}

#include "moc_kcolorschemewatcher_win.cpp"

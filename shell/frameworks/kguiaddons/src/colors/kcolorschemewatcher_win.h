/*
 * SPDX-FileCopyrightText: 2020 Piyush Aggarwal <piyushaggarwal002@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KCOLORSCHEMEWATCHER_WIN_H
#define KCOLORSCHEMEWATCHER_WIN_H

#include <QAbstractNativeEventFilter>
#include <QSettings>

#include "kcolorschemewatcherbackend.h"

class KColorSchemeWatcherWin : public KColorSchemeWatcherBackend, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    KColorSchemeWatcherWin();
    KColorSchemeWatcher::ColorPreference systemPreference() const override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

private:
    QSettings m_settings{QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"), QSettings::NativeFormat};
    bool m_preferDarkMode = false;
};

#endif

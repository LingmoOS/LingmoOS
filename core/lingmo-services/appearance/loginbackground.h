// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"

class WallpaperManager;

/**
 * Hands the desktop background to lingmo-dm, which keeps a copy the login
 * screen can read: once at login, then after every change. The picture goes
 * as an open file, so the display manager reads only what this user can.
 */
class LoginBackground
{
public:
    LoginBackground(WallpaperManager &wallpapers, Lingmo::Base::DBus::Connection *system);

    LoginBackground(const LoginBackground &) = delete;
    LoginBackground &operator=(const LoginBackground &) = delete;

private:
    void send();

    WallpaperManager &m_wallpapers;
    Lingmo::Base::DBus::Connection *m_system;
    // A wallpaper, its type and its colour often change together: one send.
    Lingmo::Base::Timer m_pending;
    std::vector<Lingmo::Base::Subscription> m_subscriptions;
};


// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loginbackground.h"

#include <fcntl.h>

#include "base/url.h"

#include "dbuscall.h"
#include "log.h"
#include "wallpapermanager.h"

using namespace Lingmo::Base;

LoginBackground::LoginBackground(WallpaperManager &wallpapers, dbus::Connection *system)
    : m_wallpapers(wallpapers)
    , m_system(system)
{
    if (!m_system)
        return;

    m_pending.setSingleShot(true);
    m_pending.onTimeout = [this] { send(); };

    const auto schedule = [this] { m_pending.start(std::chrono::nanoseconds(0)); };
    m_subscriptions.push_back(m_wallpapers.wallpaperChanged.connect([schedule](const std::string &) { schedule(); }));
    m_subscriptions.push_back(m_wallpapers.backgroundTypeChanged.connect(schedule));
    m_subscriptions.push_back(m_wallpapers.backgroundColorChanged.connect(schedule));
    schedule();
}

void LoginBackground::send()
{
    const dbus::Proxy manager(m_system, "org.lingmo.DisplayManager", "/org/lingmo/DisplayManager",
                              "org.lingmo.DisplayManager");
    // Under another display manager there is no one to answer.
    const auto done = [](const dbus::Reply &reply) {
        if (reply.isError() && reply.error().name != "org.freedesktop.DBus.Error.ServiceUnknown")
            LM_WARNING(services::log()) << "Cannot hand the background to the login screen:"
                                        << reply.error().message;
    };

    const std::string wallpaper = m_wallpapers.wallpaper();
    if (m_wallpapers.backgroundType() == 0 && !wallpaper.empty()) {
        const std::string path = url::toLocalFile(wallpaper).value_or(wallpaper);
        const dbus::UnixFd file = dbus::UnixFd::adopt(open(path.c_str(), O_RDONLY | O_CLOEXEC));
        if (file.isValid())
            services::callDetached(manager, "SetBackgroundPicture", done, file);
        return;
    }

    services::callDetached(manager, "SetBackgroundColor", done, m_wallpapers.backgroundColor());
}

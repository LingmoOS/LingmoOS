// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/file_watcher.h"
#include "base/signal.h"
#include "base/timer.h"

namespace settings {

// The applications that can be the default web browser, mail client,
// terminal and video player, and which ones are. The choices go to the
// user's mimeapps.list and the terminal list.
class DefaultApplications
{
public:
    enum Kind {
        Browser,
        Email,
        Terminal,
        VideoPlayer,
        KindCount,
    };

    struct Application
    {
        std::string path;
        // The desktop file's name, which mimeapps.list refers to.
        std::string fileName;
        std::string name;
        std::string icon;
        std::vector<std::string> mimeTypes;
    };

    DefaultApplications();

    const std::vector<Application>& applications(Kind kind) const { return m_lists[kind]; }
    // -1 when none of them is the default.
    int defaultIndex(Kind kind) const { return m_defaults[kind]; }
    void setDefault(Kind kind, int index);

    // After the lists or the defaults changed.
    Lingmo::base::Signal<> changed;

private:
    void load();
    void watchLists();

    std::vector<Application> m_lists[KindCount];
    int m_defaults[KindCount] = {-1, -1, -1, -1};
    Lingmo::base::Subscription m_registry;
    std::vector<Lingmo::base::FileWatch> m_watches;
    Lingmo::base::Timer m_reload;
};

} // namespace settings

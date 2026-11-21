// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

// The monitor's choices in ~/.config/lingmo/system-monitor.conf.
struct Preferences
{
    // Milliseconds.
    int interval = 1000;
    int scope = 1;
    std::string sortColumn = "cpu";
    bool sortAscending = false;

    // Defaults for what the file does not hold.
    static Preferences load(const std::string& path);
    // Keeps whatever else the file holds.
    void save(const std::string& path) const;
    static std::string defaultPath();
};

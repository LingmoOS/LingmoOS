// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "preferences.h"

#include "base/paths.h"
#include "base/settings.h"

#include <algorithm>

using Lingmo::base::Settings;

Preferences Preferences::load(const std::string& path)
{
    Preferences preferences;
    const Settings settings(path);
    const int64_t interval = settings.intValue("UpdateInterval", 1000);
    preferences.interval = interval == 2000 || interval == 5000 ? int(interval) : 1000;
    preferences.scope = int(std::clamp<int64_t>(settings.intValue("Processes/Scope", preferences.scope), 0, 2));
    preferences.sortColumn = settings.stringValue("Processes/SortColumn", preferences.sortColumn);
    preferences.sortAscending = settings.boolValue("Processes/SortAscending", preferences.sortAscending);
    return preferences;
}

void Preferences::save(const std::string& path) const
{
    Settings settings(path);
    settings.setValue("UpdateInterval", interval);
    settings.setValue("Processes/Scope", scope);
    settings.setValue("Processes/SortColumn", sortColumn);
    settings.setValue("Processes/SortAscending", sortAscending);
}

std::string Preferences::defaultPath()
{
    return Lingmo::base::paths::lingmoConfig("system-monitor");
}

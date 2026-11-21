// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/preferences.h"
#include "core/config.h"

#include "base/paths.h"
#include "base/settings.h"

#include <algorithm>

namespace fm {

static constexpr size_t kMaxRecentServers = 10;

Preferences& Preferences::instance()
{
    static Preferences preferences(settingsFile());
    return preferences;
}

Preferences::Preferences(const std::string& path)
    : m_settings(std::make_unique<base::Settings>(path))
{
    base::Settings& s = *m_settings;
    m_viewMethod = int(s.intValue("viewMethod", m_viewMethod));
    m_sortMode = int(s.intValue("sortMode", m_sortMode));
    m_sortDesc = s.boolValue("sortDesc", m_sortDesc);
    m_trashSortMode = int(s.intValue("trashSortMode", m_trashSortMode));
    m_trashSortDesc = s.boolValue("trashSortDesc", m_trashSortDesc);
    m_listOriginColumnWidth = s.doubleValue("listOriginColumnWidth", m_listOriginColumnWidth);
    m_listDateColumnWidth = s.doubleValue("listDateColumnWidth", m_listDateColumnWidth);
    m_listSizeColumnWidth = s.doubleValue("listSizeColumnWidth", m_listSizeColumnWidth);
    m_listTypeColumnWidth = s.doubleValue("listTypeColumnWidth", m_listTypeColumnWidth);
    m_maximumIconSize = int(s.intValue("maximumIconSize", m_maximumIconSize));
    m_minimumIconSize = int(s.intValue("minimumIconSize", m_minimumIconSize));
    m_gridIconSize = s.doubleValue("gridIconSize", m_gridIconSize);
    m_sidebarWidth = s.doubleValue("sidebarWidth", m_sidebarWidth);
    // Type sorting used to be stored as 6, which is not a model column.
    if (m_sortMode == 6)
        m_sortMode = Type;
}

Preferences::~Preferences() = default;

void Preferences::save()
{
    if (m_readOnly) {
        changed.emit();
        return;
    }
    base::Settings& s = *m_settings;
    s.setValue("viewMethod", m_viewMethod);
    s.setValue("sortMode", m_sortMode);
    s.setValue("sortDesc", m_sortDesc);
    s.setValue("trashSortMode", m_trashSortMode);
    s.setValue("trashSortDesc", m_trashSortDesc);
    s.setValue("listOriginColumnWidth", m_listOriginColumnWidth);
    s.setValue("listDateColumnWidth", m_listDateColumnWidth);
    s.setValue("listSizeColumnWidth", m_listSizeColumnWidth);
    s.setValue("listTypeColumnWidth", m_listTypeColumnWidth);
    s.setValue("maximumIconSize", m_maximumIconSize);
    s.setValue("minimumIconSize", m_minimumIconSize);
    s.setValue("gridIconSize", m_gridIconSize);
    s.setValue("sidebarWidth", m_sidebarWidth);
    s.sync();
    changed.emit();
}

void Preferences::setViewMethod(int method)
{
    if (m_viewMethod == method)
        return;
    m_viewMethod = method;
    save();
}

void Preferences::setSort(int mode, bool descending)
{
    if (m_sortMode == mode && m_sortDesc == descending)
        return;
    m_sortMode = mode;
    m_sortDesc = descending;
    save();
}

void Preferences::setTrashSort(int mode, bool descending)
{
    if (m_trashSortMode == mode && m_trashSortDesc == descending)
        return;
    m_trashSortMode = mode;
    m_trashSortDesc = descending;
    save();
}

void Preferences::setListColumnWidths(double origin, double date, double size, double type)
{
    if (m_listOriginColumnWidth == origin && m_listDateColumnWidth == date && m_listSizeColumnWidth == size
        && m_listTypeColumnWidth == type)
        return;
    m_listOriginColumnWidth = origin;
    m_listDateColumnWidth = date;
    m_listSizeColumnWidth = size;
    m_listTypeColumnWidth = type;
    save();
}

void Preferences::setGridIconSize(double size)
{
    if (m_gridIconSize == size)
        return;
    m_gridIconSize = size;
    save();
}

void Preferences::setSidebarWidth(double width)
{
    if (m_sidebarWidth == width)
        return;
    m_sidebarWidth = width;
    save();
}

std::vector<std::string> Preferences::recentServers() const
{
    m_settings->reload();
    return m_settings->stringListValue("Network/recentServers");
}

void Preferences::addRecentServer(const std::string& address)
{
    std::vector<std::string> servers = recentServers();
    std::erase(servers, address);
    servers.insert(servers.begin(), address);
    if (servers.size() > kMaxRecentServers)
        servers.resize(kMaxRecentServers);
    m_settings->setValue("Network/recentServers", servers);
    m_settings->sync();
}

} // namespace fm

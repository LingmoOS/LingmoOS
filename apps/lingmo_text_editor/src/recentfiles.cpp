// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfiles.h"

#include "base/settings.h"

#include <algorithm>
#include <filesystem>

static constexpr const char *filesKey = "Recent/Files";
static constexpr size_t maximumFiles = 20;

static std::vector<std::string> existing(const std::vector<std::string> &paths)
{
    std::vector<std::string> result;
    std::error_code error;
    for (const std::string &path : paths) {
        if (std::filesystem::exists(path, error))
            result.push_back(path);
    }
    return result;
}

RecentFiles::RecentFiles(const std::string &settingsFile)
    : m_settingsFile(settingsFile)
{
    const Lingmo::base::Settings settings(m_settingsFile);
    m_files = existing(settings.stringListValue(filesKey));
    if (m_files.size() > maximumFiles)
        m_files.resize(maximumFiles);
}

void RecentFiles::add(const std::string &path)
{
    if (!m_files.empty() && m_files.front() == path)
        return;
    std::vector<std::string> files = existing(m_files);
    std::erase(files, path);
    files.insert(files.begin(), path);
    if (files.size() > maximumFiles)
        files.resize(maximumFiles);
    m_files = std::move(files);
    write();
    filesChanged();
}

void RecentFiles::clear()
{
    if (m_files.empty())
        return;
    m_files.clear();
    write();
    filesChanged();
}

void RecentFiles::write()
{
    Lingmo::base::Settings settings(m_settingsFile);
    if (m_files.empty())
        settings.remove(filesKey);
    else
        settings.setValue(filesKey, m_files);
    settings.sync();
}

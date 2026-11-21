// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include "base/signal.h"

#include <string>
#include <vector>

// Files this application opened or saved, kept under [Recent] in its own settings file.
class RecentFiles
{
public:
    explicit RecentFiles(const std::string &settingsFile);

    // Local paths, newest first.
    const std::vector<std::string> &files() const { return m_files; }

    void add(const std::string &path);
    void clear();

    Lingmo::base::Signal<> filesChanged;

private:
    void write();

    std::string m_settingsFile;
    std::vector<std::string> m_files;
};

#endif

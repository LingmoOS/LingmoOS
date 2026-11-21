// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

namespace fm {

struct Crumb
{
    std::string name;
    // A local path, or the URL for the trash.
    std::string path;
};

// The path bar's crumbs for a folder, root first: folders under home start
// from Home, a phone or server from its own name, the rest from Computer.
// The URL comes back as a path when it is local.
std::vector<Crumb> pathCrumbs(const std::string& url, std::string* resolved = nullptr);

// PathCompletionModel: the folders that complete what is typed in a path
// editor; it lists a directory once and filters it as the text grows.
class PathCompletion
{
public:
    void setText(const std::string& text);
    const std::string& text() const { return m_text; }
    int count() const { return int(m_matches.size()); }
    const std::string& name(int row) const { return m_matches[size_t(row)]; }
    // What all matches share, appended to the typed folder.
    std::string commonCompletion() const;
    std::string completion(int row) const;
    std::string path(int row) const;

private:
    void update();

    std::string m_text;
    std::string m_typedDir;
    std::string m_dir;
    std::string m_listedDir;
    std::vector<std::string> m_entries;
    std::vector<std::string> m_matches;
};

} // namespace fm

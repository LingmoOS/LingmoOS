// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/pathbarmodel.h"
#include "core/common.h"
#include "core/mounts.h"
#include "core/standardfolders.h"

#include "base/collator.h"
#include "base/i18n.h"
#include "base/strings.h"
#include "base/url.h"

#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

using Lingmo::i18n::tr;

namespace fm {

std::vector<Crumb> pathCrumbs(const std::string& url, std::string* resolved)
{
    std::string path = url;
    if (const auto local = base::url::toLocalFile(url))
        path = *local;
    if (resolved)
        *resolved = path;

    std::vector<Crumb> crumbs;
    if (path.starts_with("computer:/")) {
        crumbs.push_back({tr("Computer"), "computer:///"});
        return crumbs;
    }
    if (path.starts_with("trash:/")) {
        crumbs.push_back({path, path});
        return crumbs;
    }

    const std::string home = homePath();
    std::string directory = cleanPath(path.starts_with('/') ? path : "/" + path);
    while (true) {
        if (home != "/" && directory == home) {
            crumbs.push_back({standardFolderName(home), home});
            break;
        }
        // A phone or server starts from its own name, not the folder it is mounted at.
        const MountsClient::Mount mount = MountsClient::instance().mountAt(directory);
        if (!mount.path.empty() && directory == mount.path) {
            crumbs.push_back({mount.name, mount.path});
            break;
        }
        if (directory == "/") {
            crumbs.push_back({tr("Computer"), "/"});
            break;
        }
        const std::string standard = standardFolderName(directory);
        crumbs.push_back({standard.empty() ? fileName(directory) : standard, directory});
        directory = parentPath(directory);
    }
    std::reverse(crumbs.begin(), crumbs.end());
    return crumbs;
}

void PathCompletion::setText(const std::string& text)
{
    if (m_text == text)
        return;
    m_text = text;
    update();
}

std::string PathCompletion::commonCompletion() const
{
    if (m_matches.empty())
        return {};
    const std::string& first = m_matches.front();
    size_t length = first.size();
    for (const std::string& match : m_matches) {
        size_t i = 0;
        while (i < length && i < match.size()
               && base::strings::toAsciiLower(first[i]) == base::strings::toAsciiLower(match[i]))
            ++i;
        length = i;
    }
    return m_typedDir + first.substr(0, length);
}

std::string PathCompletion::completion(int row) const
{
    return row >= 0 && row < count() ? m_typedDir + m_matches[size_t(row)] : std::string();
}

std::string PathCompletion::path(int row) const
{
    return row >= 0 && row < count() ? cleanPath(m_dir + '/' + m_matches[size_t(row)]) : std::string();
}

void PathCompletion::update()
{
    m_matches.clear();
    const size_t slash = m_text.rfind('/');
    m_typedDir = slash == std::string::npos ? std::string() : m_text.substr(0, slash + 1);
    const std::string prefix = slash == std::string::npos ? m_text : m_text.substr(slash + 1);
    if (m_typedDir.starts_with("~/"))
        m_dir = homePath() + m_typedDir.substr(1);
    else if (m_typedDir.starts_with("file://"))
        m_dir = base::url::toLocalFile(m_typedDir).value_or(std::string());
    else if (m_typedDir.starts_with('/'))
        m_dir = m_typedDir;
    else
        m_dir.clear();

    // Folders made since the editor opened show up the next time it does.
    if (m_dir.empty()) {
        m_listedDir.clear();
        return;
    }

    // Every keystroke refilters; only a new directory part lists again.
    if (m_dir != m_listedDir) {
        m_entries.clear();
        if (DIR* directory = ::opendir(m_dir.c_str())) {
            while (dirent* entry = ::readdir(directory)) {
                const std::string name = entry->d_name;
                if (name == "." || name == "..")
                    continue;
                struct stat st;
                if (::stat((m_dir + '/' + name).c_str(), &st) == 0 && S_ISDIR(st.st_mode))
                    m_entries.push_back(name);
            }
            ::closedir(directory);
        }
        base::Collator collator;
        collator.setNumericMode(true);
        collator.setCaseSensitive(false);
        std::sort(m_entries.begin(), m_entries.end(), collator);
        m_listedDir = m_dir;
    }

    // Hidden folders only once a dot asks for them.
    const bool hidden = prefix.starts_with('.');
    for (const std::string& entry : m_entries) {
        if (entry.starts_with('.') != hidden)
            continue;
        if (base::strings::startsWith(entry, prefix, base::strings::CaseSensitivity::Insensitive))
            m_matches.push_back(entry);
    }
    // Nothing left to complete once the name is typed out.
    if (m_matches.size() == 1 && m_matches.front() == prefix)
        m_matches.clear();
}

} // namespace fm

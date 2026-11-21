// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/standardfolders.h"

#include "base/i18n.h"
#include "base/paths.h"

#include <memory>
#include <mutex>
#include <vector>

using Lingmo::base::paths::UserDirectory;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

namespace fm {

namespace {

struct StandardFolder
{
    std::string name;
    std::string icon;
};

} // namespace

static const std::map<std::string, StandardFolder>& standardFolders()
{
    static const std::map<std::string, StandardFolder> folders = [] {
        using Lingmo::base::paths::userDirectory;
        const std::vector<std::pair<std::string, StandardFolder>> list = {
            {homePath(), {trNoop("Home"), "folder-home"}},
            {userDirectory(UserDirectory::Desktop), {trNoop("Desktop"), "folder-desktop"}},
            {userDirectory(UserDirectory::Documents), {trNoop("Documents"), "folder-documents"}},
            {userDirectory(UserDirectory::Downloads), {trNoop("Downloads"), "folder-download"}},
            {userDirectory(UserDirectory::Music), {trNoop("Music"), "folder-music"}},
            {userDirectory(UserDirectory::Pictures), {trNoop("Pictures"), "folder-picture"}},
            {userDirectory(UserDirectory::Videos), {trNoop("Videos"), "folder-video"}},
            {userDirectory(UserDirectory::Templates), {trNoop("Templates"), "folder-templates"}},
            {userDirectory(UserDirectory::PublicShare), {trNoop("Public"), "folder-publicshare"}},
        };
        std::map<std::string, StandardFolder> result;
        // Home first: a disabled XDG folder resolves to home, which keeps its own name.
        for (const auto& [path, folder] : list) {
            if (!path.empty())
                result.emplace(cleanPath(path), folder);
        }
        return result;
    }();
    return folders;
}

// Read by the sorting workers as well as the UI thread.
static std::mutex s_namesMutex;
static std::shared_ptr<const std::map<std::string, std::string>> s_names;

std::shared_ptr<const std::map<std::string, std::string>> standardFolderNames()
{
    const std::lock_guard lock(s_namesMutex);
    if (!s_names) {
        auto names = std::make_shared<std::map<std::string, std::string>>();
        for (const auto& [path, folder] : standardFolders())
            names->emplace(path, tr(folder.name.c_str()));
        s_names = std::move(names);
    }
    return s_names;
}

void forgetStandardFolderNames()
{
    const std::lock_guard lock(s_namesMutex);
    s_names.reset();
}

std::string standardFolderName(const std::string& path)
{
    if (path.empty())
        return {};
    const auto found = standardFolders().find(cleanPath(path));
    return found != standardFolders().end() ? std::string(tr(found->second.name.c_str())) : std::string();
}

std::string standardFolderIconName(const std::string& path)
{
    if (path.empty())
        return {};
    const auto found = standardFolders().find(cleanPath(path));
    return found != standardFolders().end() ? found->second.icon : std::string();
}

bool isStandardFolder(const std::string& path)
{
    return !standardFolderIconName(path).empty();
}

bool canMoveStandardFolderTo(const std::string& sourcePath, const std::string& destinationPath)
{
    if (!isStandardFolder(sourcePath))
        return true;

    const std::string home = homePath();
    return cleanPath(sourcePath) != home && cleanPath(destinationPath) == home;
}

std::string cleanPath(const std::string& path)
{
    if (path.empty())
        return path;

    const bool absolute = path.front() == '/';
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= path.size()) {
        const size_t end = std::min(path.find('/', start), path.size());
        const std::string part = path.substr(start, end - start);
        if (part == "..") {
            if (!parts.empty() && parts.back() != "..")
                parts.pop_back();
            else if (!absolute)
                parts.push_back(part);
        } else if (!part.empty() && part != ".") {
            parts.push_back(part);
        }
        start = end + 1;
    }

    std::string result = absolute ? "/" : "";
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i)
            result += '/';
        result += parts[i];
    }
    return result.empty() ? "." : result;
}

std::string fileName(const std::string& path)
{
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string parentPath(const std::string& path)
{
    const std::string clean = cleanPath(path);
    const size_t slash = clean.rfind('/');
    if (slash == std::string::npos)
        return ".";
    return slash == 0 ? "/" : clean.substr(0, slash);
}

std::string homePath()
{
    static const std::string home = cleanPath(Lingmo::base::paths::home());
    return home;
}

} // namespace fm

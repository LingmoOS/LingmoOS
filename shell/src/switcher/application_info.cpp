// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "switcher/application_info.h"

#include <algorithm>
#include <filesystem>
#include <sys/stat.h>

#include "base/application_registry.h"
#include "base/file.h"
#include "base/locale.h"
#include "base/strings.h"
#include <string_view>

namespace Lingmo::Switcher {

namespace {

struct DesktopEntry {
    std::string name;
    std::string icon;
    std::string startupWmClass;
    bool valid = false;
};

std::string trimmed(std::string_view text)
{
    return std::string(base::strings::trimmed(text));
}

bool isFile(const std::string &path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

// The [Desktop Entry] group's Name in the user's language, Icon and
// StartupWMClass.
DesktopEntry readDesktopEntry(const std::string &path)
{
    const std::optional<std::string> contents = base::file::read(path);
    if (!contents)
        return {};
    const std::string locale = base::Locale::system().name();
    const std::string language = locale.substr(0, locale.find('_'));
    DesktopEntry entry;
    std::string localized;
    std::string languageOnly;
    bool inGroup = false;
    size_t start = 0;
    while (start < contents->size()) {
        size_t end = contents->find('\n', start);
        if (end == std::string::npos)
            end = contents->size();
        const std::string line = trimmed(std::string_view(*contents).substr(start, end - start));
        start = end + 1;
        if (line.empty() || line.front() == '#')
            continue;
        if (line.front() == '[') {
            if (inGroup)
                break;
            inGroup = line == "[Desktop Entry]";
            continue;
        }
        const size_t separator = line.find('=');
        if (!inGroup || separator == 0 || separator == std::string::npos)
            continue;
        const std::string key = trimmed(std::string_view(line).substr(0, separator));
        const std::string value = trimmed(std::string_view(line).substr(separator + 1));
        if (key == "Name")
            entry.name = value;
        else if (key == "Name[" + locale + "]")
            localized = value;
        else if (key == "Name[" + language + "]")
            languageOnly = value;
        else if (key == "Icon")
            entry.icon = value;
        else if (key == "StartupWMClass")
            entry.startupWmClass = value;
    }
    if (!localized.empty())
        entry.name = localized;
    else if (!languageOnly.empty())
        entry.name = languageOnly;
    entry.valid = !entry.name.empty();
    return entry;
}

// The directory's *.desktop files, sorted by name ignoring case.
std::vector<std::string> desktopFiles(const std::string &directory)
{
    std::vector<std::string> names;
    std::error_code error;
    for (const auto &item : std::filesystem::directory_iterator(directory, error)) {
        const std::string name = item.path().filename().string();
        if (name.starts_with('.') || !name.ends_with(".desktop") || !isFile(item.path().string()))
            continue;
        names.push_back(name);
    }
    std::sort(names.begin(), names.end(), base::strings::LessIgnoreAsciiCase());
    return names;
}

} // namespace

ApplicationInfoCache::ApplicationInfoCache()
    : ApplicationInfoCache(base::ApplicationRegistry::desktopPaths())
{
}

ApplicationInfoCache::ApplicationInfoCache(std::vector<std::string> directories)
    : m_directories(std::move(directories))
{
}

// Installing or removing an application changes its directory.
int64_t ApplicationInfoCache::applicationsStamp() const
{
    int64_t stamp = 0;
    for (const std::string &directory : m_directories) {
        struct stat info;
        if (stat(directory.c_str(), &info) == 0)
            stamp = std::max(stamp, int64_t(info.st_mtim.tv_sec) * 1000
                                            + info.st_mtim.tv_nsec / 1000000);
    }
    return stamp;
}

void ApplicationInfoCache::refresh()
{
    const int64_t stamp = applicationsStamp();
    if (stamp == m_stamp)
        return;
    m_stamp = stamp;
    m_cache.clear();
    m_wmClasses.reset();
}

ApplicationInfo ApplicationInfoCache::lookup(const std::string &applicationId,
                                             const std::string &fallbackName)
{
    if (applicationId.empty())
        return {fallbackName, kFallbackIcon};
    if (m_stamp < 0)
        refresh();
    if (const ApplicationInfo *cached = m_cache.find(applicationId))
        return *cached;

    DesktopEntry entry;
    for (const std::string &candidate : {applicationId, base::strings::toAsciiLower(applicationId)}) {
        for (const std::string &directory : m_directories) {
            const std::string path = directory + '/' + candidate + ".desktop";
            if (isFile(path)) {
                entry = readDesktopEntry(path);
                break;
            }
        }
        if (entry.valid)
            break;
    }
    // An X11 class, or a Wayland id that is not the entry's file name.
    if (!entry.valid) {
        const auto &classes = wmClasses();
        if (const auto found = classes.find(base::strings::toAsciiLower(applicationId)); found != classes.end())
            entry = readDesktopEntry(found->second);
    }

    ApplicationInfo info;
    info.name = entry.valid ? entry.name : applicationId;
    info.icon = entry.valid && !entry.icon.empty() ? entry.icon : kFallbackIcon;
    m_cache.insert(applicationId, std::make_unique<ApplicationInfo>(info));
    return info;
}

const std::unordered_map<std::string, std::string> &ApplicationInfoCache::wmClasses()
{
    if (m_wmClasses)
        return *m_wmClasses;
    m_wmClasses.emplace();
    // Reversed, so an entry in an earlier directory shadows a later one.
    for (auto directory = m_directories.rbegin(); directory != m_directories.rend(); ++directory) {
        for (const std::string &file : desktopFiles(*directory)) {
            const std::string path = *directory + '/' + file;
            const DesktopEntry entry = readDesktopEntry(path);
            if (entry.valid && !entry.startupWmClass.empty())
                (*m_wmClasses)[base::strings::toAsciiLower(entry.startupWmClass)] = path;
        }
    }
    return *m_wmClasses;
}

} // namespace Lingmo::Switcher

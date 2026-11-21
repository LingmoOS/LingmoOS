// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/mime_icons.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <vector>

#include "base/file.h"
#include "base/paths.h"
#include "base/strings.h"
#include <string_view>

namespace Lingmo::Shell {

namespace {

struct Glob {
    int weight = 50;
    std::string type;
    std::string pattern;
    bool caseSensitive = false;
};

struct MimeData {
    std::vector<Glob> globs;
    std::map<std::string, std::string> icons;
};

} // namespace

static std::vector<std::string> mimeDirectories()
{
    std::vector<std::string> directories{base::paths::dataHome() + "/mime"};
    for (const std::string &directory : base::paths::dataDirectories())
        directories.push_back(directory + "/mime");
    return directories;
}

// Read once; the user's directory first, as it overrides the system's.
static const MimeData &mimeData()
{
    static std::once_flag once;
    static MimeData data;
    std::call_once(once, [] {
        for (const std::string &directory : mimeDirectories()) {
            if (const auto globs = base::file::read(directory + "/globs2")) {
                for (const std::string &line : base::strings::split(*globs, '\n')) {
                    if (line.empty() || line.front() == '#')
                        continue;
                    const std::vector<std::string> fields = base::strings::split(line, ':');
                    if (fields.size() < 3)
                        continue;
                    Glob glob;
                    glob.weight = base::strings::toNumber<int>(fields[0]).value_or(50);
                    glob.type = fields[1];
                    glob.pattern = fields[2];
                    glob.caseSensitive = fields.size() > 3 && fields[3].find("cs") != std::string::npos;
                    data.globs.push_back(std::move(glob));
                }
            }
            if (const auto icons = base::file::read(directory + "/icons")) {
                for (const std::string &line : base::strings::split(*icons, '\n')) {
                    const size_t colon = line.find(':');
                    if (colon != std::string::npos)
                        data.icons.try_emplace(line.substr(0, colon), line.substr(colon + 1));
                }
            }
        }
    });
    return data;
}

// "*.ext" and plain names; other patterns are rare in the database.
static bool matches(const Glob &glob, const std::string &name)
{
    const std::string subject = glob.caseSensitive ? name : base::strings::toAsciiLower(name);
    const std::string pattern = glob.caseSensitive ? glob.pattern : base::strings::toAsciiLower(glob.pattern);
    if (pattern.starts_with('*') && pattern.find_first_of("*?[", 1) == std::string::npos)
        return subject.size() >= pattern.size() - 1 && subject.ends_with(std::string_view(pattern).substr(1));
    if (pattern.find_first_of("*?[") == std::string::npos)
        return subject == pattern;
    return false;
}

static std::string iconOf(const std::string &type)
{
    const MimeData &data = mimeData();
    const auto it = data.icons.find(type);
    if (it != data.icons.end())
        return it->second;
    return base::strings::replaceAll(type, "/", "-");
}

std::string mimeIconNameForFile(const std::string &path)
{
    std::error_code error;
    if (std::filesystem::is_directory(path, error))
        return iconOf("inode/directory");

    const std::string name = std::filesystem::path(path).filename().string();
    const Glob *best = nullptr;
    for (const Glob &glob : mimeData().globs) {
        if (!matches(glob, name))
            continue;
        if (!best || glob.weight > best->weight
            || (glob.weight == best->weight && glob.pattern.size() > best->pattern.size()))
            best = &glob;
    }
    if (best)
        return iconOf(best->type);

    // No name says it: text when its start reads as UTF-8 without NULs.
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return iconOf("application/octet-stream");
    char buffer[512];
    file.read(buffer, sizeof(buffer));
    const std::string_view start(buffer, size_t(file.gcount()));
    if (start.find('\0') == std::string_view::npos && base::strings::isValidUtf8(start))
        return iconOf("text/plain");
    return iconOf("application/octet-stream");
}

} // namespace Lingmo::Shell

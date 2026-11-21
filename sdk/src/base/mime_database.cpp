// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/mime_database.h"

#include <algorithm>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>

#include "base/file.h"
#include "base/paths.h"

namespace Lingmo::base::mime {

namespace {

struct Database
{
    std::set<std::string, std::less<>> types;
    // Alias to type.
    std::unordered_map<std::string, std::string> canonical;
    std::unordered_map<std::string, std::vector<std::string>> aliases;
    std::unordered_map<std::string, std::vector<std::string>> parents;
};

} // namespace

static void appendIfNew(std::vector<std::string>& list, const std::string& value)
{
    if (std::find(list.begin(), list.end(), value) == list.end())
        list.push_back(value);
}

// "first second" pairs, one per line, as update-mime-database writes them.
static std::vector<std::pair<std::string, std::string>> pairs(const std::string& path)
{
    std::vector<std::pair<std::string, std::string>> result;
    const std::optional<std::string> contents = file::read(path);
    if (!contents)
        return result;
    std::istringstream lines(*contents);
    std::string line;
    while (std::getline(lines, line)) {
        const size_t space = line.find(' ');
        if (line.empty() || line[0] == '#' || space == std::string::npos)
            continue;
        result.emplace_back(line.substr(0, space), line.substr(space + 1));
    }
    return result;
}

static const Database& database()
{
    static const Database database = [] {
        Database result;
        // The user's types first, so they override the system's.
        std::vector<std::string> directories = {paths::dataHome() + "/mime"};
        for (const std::string& directory : paths::dataDirectories())
            directories.push_back(directory + "/mime");
        for (const std::string& directory : directories) {
            if (const std::optional<std::string> types = file::read(directory + "/types")) {
                std::istringstream lines(*types);
                std::string line;
                while (std::getline(lines, line)) {
                    if (!line.empty())
                        result.types.insert(line);
                }
            }
            for (const auto& [alias, type] : pairs(directory + "/aliases")) {
                if (result.canonical.emplace(alias, type).second)
                    appendIfNew(result.aliases[type], alias);
            }
            for (const auto& [type, parent] : pairs(directory + "/subclasses"))
                appendIfNew(result.parents[type], parent);
        }
        return result;
    }();
    return database;
}

std::string canonicalName(std::string_view name)
{
    const Database& db = database();
    const auto alias = db.canonical.find(std::string(name));
    const std::string type = alias != db.canonical.end() ? alias->second : std::string(name);
    return db.types.count(type) ? type : std::string();
}

bool isKnown(std::string_view name)
{
    return !canonicalName(name).empty();
}

std::vector<std::string> aliases(std::string_view name)
{
    const Database& db = database();
    const auto it = db.aliases.find(canonicalName(name));
    return it == db.aliases.end() ? std::vector<std::string>() : it->second;
}

std::vector<std::string> parents(std::string_view name)
{
    // An unknown name still gets the fallback parent below.
    const std::string known = canonicalName(name);
    const std::string type = known.empty() ? std::string(name) : known;
    const Database& db = database();
    const auto it = db.parents.find(type);
    if (it != db.parents.end() && !it->second.empty())
        return it->second;
    // Text types without a parent are text/plain's, other file types
    // application/octet-stream's.
    const std::string group = type.substr(0, type.find('/'));
    if (group == "text" && type != "text/plain")
        return {"text/plain"};
    if (group != "inode" && group != "all" && group != "fonts" && group != "print" && group != "uri"
        && type != "application/octet-stream")
        return {"application/octet-stream"};
    return {};
}

} // namespace Lingmo::base::mime

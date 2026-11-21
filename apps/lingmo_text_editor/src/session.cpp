// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "session.h"

#include "base/file.h"
#include "base/json.h"
#include "base/paths.h"

#include <algorithm>
#include <chrono>
#include <filesystem>

using Lingmo::base::json::Value;

// A second early: file times come from a coarser clock and can lag behind it.
static const std::filesystem::file_time_type s_processStart =
        std::filesystem::file_time_type::clock::now() - std::chrono::seconds(1);

// "<id>.txt.XXXXXX": a draft's temporary file, as mkostemp() names it.
static bool isDraftTemporary(const std::string &name)
{
    const size_t dot = name.rfind('.');
    if (dot == std::string::npos || name.size() - dot != 7 || !name.substr(0, dot).ends_with(".txt"))
        return false;
    return std::all_of(name.begin() + long(dot) + 1, name.end(), [](char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    });
}

static std::string &dataDirectoryOverride()
{
    static std::string directory;
    return directory;
}

std::string Session::dataDirectory()
{
    const std::string &directory = dataDirectoryOverride();
    return directory.empty() ? Lingmo::base::paths::dataHome() + "/lingmo-text-editor" : directory;
}

void Session::setDataDirectory(const std::string &directory)
{
    dataDirectoryOverride() = directory;
}

std::string Session::defaultPath()
{
    return dataDirectory() + "/session.json";
}

bool Session::save(const std::vector<WindowState> &windows, const std::string &path)
{
    Value windowArray = Value::Array();
    for (const WindowState &window : windows) {
        Value tabs = Value::Array();
        for (const TabState &tab : window.tabs) {
            Value object = Value::object({
                    {"draft", tab.draftId},
                    {"line", tab.line},
                    {"column", tab.column},
                    {"scroll", tab.scroll},
            });
            if (!tab.path.empty())
                object.insert("path", tab.path);
            if (tab.unsaved)
                object.insert("unsaved", true);
            if (!tab.encoding.empty())
                object.insert("encoding", tab.encoding);
            if (!tab.language.empty())
                object.insert("language", tab.language);
            tabs.append(std::move(object));
        }
        if (tabs.size() == 0)
            continue;
        windowArray.append(Value::object({{"tabs", std::move(tabs)}, {"current", window.current}}));
    }

    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), error);
    const Value root = Value::object({{"windows", std::move(windowArray)}});
    return Lingmo::base::file::writeAtomically(path, root.serialize());
}

std::vector<WindowState> Session::load(const std::string &path, bool *ok)
{
    std::vector<WindowState> result;
    if (ok)
        *ok = true;
    const std::optional<std::string> text = Lingmo::base::file::read(path);
    if (!text) {
        std::error_code error;
        if (ok)
            *ok = !std::filesystem::exists(path, error);
        return result;
    }
    const std::optional<Value> document = Value::parse(*text);
    if (!document || !document->isObject()) {
        if (ok)
            *ok = false;
        return result;
    }
    for (const Value &windowValue : (*document)["windows"].toArray()) {
        WindowState window;
        for (const Value &object : windowValue["tabs"].toArray()) {
            TabState tab;
            tab.path = object["path"].toString();
            tab.draftId = object["draft"].toString();
            tab.unsaved = object["unsaved"].toBool();
            tab.line = int(object["line"].toInt());
            tab.column = int(object["column"].toInt());
            tab.scroll = object["scroll"].toDouble();
            tab.encoding = object["encoding"].toString();
            tab.language = object["language"].toString();
            // An untitled tab without a draft had nothing in it.
            if (tab.path.empty() && !tab.unsaved)
                continue;
            window.tabs.push_back(std::move(tab));
        }
        window.current = std::clamp(int(windowValue["current"].toInt()), 0, std::max(0, int(window.tabs.size()) - 1));
        if (!window.tabs.empty())
            result.push_back(std::move(window));
    }
    return result;
}

void Session::remove(const std::string &path)
{
    std::error_code error;
    std::filesystem::remove(path, error);
}

void Session::removeOrphanDrafts(const std::string &directory, const std::set<std::string> &keep, int minimumAgeDays)
{
    namespace fs = std::filesystem;
    std::error_code error;
    const auto limit = fs::file_time_type::clock::now() - std::chrono::hours(24 * minimumAgeDays);
    for (const fs::directory_entry &entry : fs::directory_iterator(directory, error)) {
        const fs::path &path = entry.path();
        if (!entry.is_regular_file(error))
            continue;
        // Left by a write that never finished; this process's own may be under way.
        if (isDraftTemporary(path.filename().string())) {
            if (entry.last_write_time(error) < s_processStart)
                fs::remove(path, error);
            continue;
        }
        if (path.extension() != ".txt")
            continue;
        if (keep.count(path.stem().string()))
            continue;
        if (minimumAgeDays == 0 || entry.last_write_time(error) < limit)
            fs::remove(path, error);
    }
}

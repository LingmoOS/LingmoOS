// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processhelper.h"

#include <filesystem>
#include <unistd.h>

#include "base/dbus.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/strings.h"
#include "base/url.h"

namespace strings = Lingmo::base::strings;

namespace ProcessHelper
{

// An absolute or home-relative path becomes a file URL, anything else stays a link.
static std::string fromUserInput(const std::string& text)
{
    const std::string trimmed(strings::trimmed(text));
    if (trimmed.starts_with('/'))
        return Lingmo::base::url::fromLocalFile(trimmed);
    if (trimmed.starts_with("~/"))
        return Lingmo::base::url::fromLocalFile(Lingmo::base::paths::home() + trimmed.substr(1));
    return trimmed;
}

bool openUrl(const std::string& url)
{
    if (url.empty())
        return false;
    return Lingmo::base::Process::startDetached("xdg-open", {fromUserInput(url)}) > 0;
}

// Asynchronous, so a file manager that is slow to start never stalls the terminal.
static void callFileManager(const char* method, const std::string& path)
{
    Lingmo::base::dbus::Connection* bus = Lingmo::base::dbus::Connection::session();
    if (!bus)
        return;
    Lingmo::base::dbus::Proxy proxy(bus, "org.freedesktop.FileManager1", "/org/freedesktop/FileManager1",
                                      "org.freedesktop.FileManager1");
    proxy.send(method, std::vector<std::string>{Lingmo::base::url::fromLocalFile(path)},
               std::to_string(getpid()));
}

bool openFileManager(const std::string& path)
{
    callFileManager("ShowFolders", path);
    return true;
}

bool showInFileManager(const std::string& path)
{
    callFileManager("ShowItems", path);
    return true;
}

std::string existingPath(const std::string& text, const std::string& baseDir)
{
    std::string path(strings::trimmed(text));

    if (path.empty() || path.find('\n') != std::string::npos || strings::utf16Length(path) > 4096)
        return std::string();

    // Shells print quoted names for paths with spaces.
    if (path.size() > 1 && (path.front() == '\'' || path.front() == '"') && path.back() == path.front())
        path = path.substr(1, path.size() - 2);

    if (path == "~" || path.starts_with("~/"))
        path.replace(0, 1, Lingmo::base::paths::home());

    // Relative to baseDir, cleaned lexically; symbolic links are not resolved.
    std::error_code error;
    const std::filesystem::path base = baseDir.empty() ? std::filesystem::current_path(error) : std::filesystem::path(baseDir);
    std::filesystem::path full = std::filesystem::path(path).is_absolute() ? std::filesystem::path(path) : base / path;
    full = full.lexically_normal();
    std::string result = full.string();
    while (result.size() > 1 && result.back() == '/')
        result.pop_back();

    return std::filesystem::exists(full, error) ? result : std::string();
}

}

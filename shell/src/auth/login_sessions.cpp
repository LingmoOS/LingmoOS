// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "auth/login_sessions.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <pwd.h>
#include <set>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

#include "base/file.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/strings.h"
#include <string_view>

namespace Lingmo::Auth {

namespace {

const std::string kStatePath = "/var/lib/lingmo-dm/state.conf";
const std::string kBackgroundDirectory = "/var/lib/lingmo-dm/backgrounds";

bool isSpace(char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

std::string trimmed(std::string_view text)
{
    return std::string(base::strings::trimmed(text));
}

std::vector<std::string> lines(const std::string &contents)
{
    std::vector<std::string> result;
    size_t start = 0;
    while (start < contents.size()) {
        size_t end = contents.find('\n', start);
        if (end == std::string::npos)
            end = contents.size();
        result.emplace_back(contents, start, end - start);
        start = end + 1;
    }
    return result;
}

std::string unescape(const std::string &value)
{
    std::string result;
    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c != '\\' || i + 1 == value.size()) {
            result += c;
            continue;
        }
        const char next = value[++i];
        result += next == 's' ? ' ' : next == 'n' ? '\n' : next == 't' ? '\t' : next;
    }
    return result;
}

bool executableExists(const std::string &program)
{
    if (program.starts_with('/')) {
        struct stat info;
        return stat(program.c_str(), &info) == 0 && access(program.c_str(), X_OK) == 0;
    }
    return !base::paths::findExecutable(program).empty();
}

// The file name without its last extension.
std::string completeBaseName(const std::string &path)
{
    std::string name = std::filesystem::path(path).filename().string();
    const size_t dot = name.rfind('.');
    return dot == std::string::npos ? name : name.substr(0, dot);
}

bool readEntry(const std::string &path, LoginSession *session)
{
    const std::optional<std::string> contents = base::file::read(path);
    if (!contents)
        return false;
    std::map<std::string, std::string> keys;
    bool inEntry = false;
    for (const std::string &raw : lines(*contents)) {
        const std::string line = trimmed(raw);
        if (line.empty() || line.starts_with('#'))
            continue;
        if (line.starts_with('[')) {
            inEntry = line == "[Desktop Entry]";
            continue;
        }
        const size_t equals = line.find('=');
        if (!inEntry || equals == 0 || equals == std::string::npos)
            continue;
        // The first of a repeated key counts.
        keys.emplace(trimmed(std::string_view(line).substr(0, equals)),
                     trimmed(std::string_view(line).substr(equals + 1)));
    }
    const auto value = [&keys](const std::string &key, const std::string &fallback = {}) {
        const auto found = keys.find(key);
        return found == keys.end() ? fallback : found->second;
    };

    if (value("Hidden") == "true" || value("NoDisplay") == "true")
        return false;
    const std::string tryExec = unescape(value("TryExec"));
    if (!tryExec.empty() && !executableExists(tryExec))
        return false;
    session->id = completeBaseName(path);
    session->name = unescape(value("Name", session->id));
    session->command.clear();
    for (const std::string &argument : splitCommand(unescape(value("Exec")))) {
        if (!(argument.size() == 2 && argument.starts_with('%')))
            session->command.push_back(argument);
    }
    std::vector<std::string> desktopNames;
    const std::string desktopNamesValue = value("DesktopNames");
    for (const std::string_view name : base::strings::splitViews(desktopNamesValue, ';')) {
        if (!base::strings::trimmed(name).empty())
            desktopNames.emplace_back(base::strings::trimmed(name));
    }
    session->environment = {"XDG_SESSION_TYPE=wayland", "DESKTOP_SESSION=" + session->id,
                            "XDG_SESSION_DESKTOP="
                                    + (desktopNames.empty() ? session->id : desktopNames.front())};
    if (!desktopNames.empty())
        session->environment.push_back("XDG_CURRENT_DESKTOP="
                                       + base::strings::join(desktopNames, ":"));
    return !session->command.empty();
}

// The directory's visible *.desktop files, sorted by name.
std::vector<std::string> desktopFiles(const std::string &directory)
{
    std::vector<std::string> files;
    std::error_code error;
    for (const auto &item : std::filesystem::directory_iterator(directory, error)) {
        const std::string name = item.path().filename().string();
        struct stat info;
        if (name.starts_with('.') || !name.ends_with(".desktop")
            || stat(item.path().c_str(), &info) != 0 || !S_ISREG(info.st_mode))
            continue;
        files.push_back(name);
    }
    std::sort(files.begin(), files.end());
    for (std::string &file : files)
        file = std::filesystem::absolute(std::filesystem::path(directory) / file).lexically_normal().string();
    return files;
}

std::string localName(const char *name)
{
    return name ? std::string(name) : std::string();
}

} // namespace

std::vector<std::string> splitCommand(std::string_view command)
{
    std::vector<std::string> arguments;
    std::string argument;
    int quotes = 0;
    bool inQuote = false;
    for (const char c : command) {
        if (c == '"') {
            ++quotes;
            if (quotes == 3) {
                quotes = 0;
                argument += c;
            }
            continue;
        }
        if (quotes) {
            if (quotes == 1)
                inQuote = !inQuote;
            quotes = 0;
        }
        if (!inQuote && isSpace(c)) {
            if (!argument.empty())
                arguments.push_back(std::exchange(argument, {}));
        } else {
            argument += c;
        }
    }
    if (!argument.empty())
        arguments.push_back(argument);
    return arguments;
}

std::vector<LoginSession> findLoginSessions(const std::vector<std::string> &directories)
{
    std::vector<LoginSession> sessions;
    std::set<std::string> seen;
    for (const std::string &directory : directories) {
        for (const std::string &file : desktopFiles(directory)) {
            if (!seen.insert(completeBaseName(file)).second)
                continue;
            LoginSession session;
            if (readEntry(file, &session))
                sessions.push_back(session);
        }
    }
    return sessions;
}

namespace {

std::vector<std::string> &testSessionDirectories()
{
    static std::vector<std::string> directories;
    return directories;
}

} // namespace

std::vector<std::string> loginSessionDirectories()
{
    if (!testSessionDirectories().empty())
        return testSessionDirectories();
    return {"/usr/local/share/wayland-sessions", "/usr/share/wayland-sessions"};
}

void setLoginSessionDirectoriesForTesting(std::vector<std::string> directories)
{
    testSessionDirectories() = std::move(directories);
}

std::vector<std::string> findLoginUsers()
{
    uid_t minimum = 1000, maximum = 60000;
    if (const std::optional<std::string> defs = base::file::read("/etc/login.defs")) {
        for (const std::string &line : lines(*defs)) {
            const std::vector<std::string> fields = base::strings::splitWhitespace(line);
            if (fields.size() != 2)
                continue;
            // toUInt(): zero for anything that is not a whole number.
            const uid_t value = base::strings::toNumber<uid_t>(fields[1]).value_or(0);
            if (fields[0] == "UID_MIN")
                minimum = value;
            else if (fields[0] == "UID_MAX")
                maximum = value;
        }
    }
    std::vector<std::pair<uid_t, std::string>> accounts;
    setpwent();
    while (const passwd *entry = getpwent()) {
        const std::string shell = localName(entry->pw_shell);
        if (entry->pw_uid < minimum || entry->pw_uid > maximum || shell.ends_with("/nologin")
            || shell.ends_with("/false"))
            continue;
        accounts.emplace_back(entry->pw_uid, localName(entry->pw_name));
    }
    endpwent();
    std::sort(accounts.begin(), accounts.end());
    std::vector<std::string> users;
    for (const auto &account : accounts)
        users.push_back(account.second);
    return users;
}

std::string lastLoginUser()
{
    return base::Settings(kStatePath).stringValue("Last/User");
}

std::string lastLoginSession(const std::string &user)
{
    return base::Settings(kStatePath).stringValue("Sessions/" + user);
}

std::string loginBackgroundFile(const std::string &user)
{
    return kBackgroundDirectory + '/' + user + ".conf";
}

} // namespace Lingmo::Auth

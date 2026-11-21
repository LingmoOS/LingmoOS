#include "sessions.h"
#include "config.h"

#include "base/file.h"
#include "base/paths.h"
#include "base/strings.h"

#include <algorithm>
#include <dirent.h>
#include <map>
#include <set>
#include <sys/stat.h>
#include <unistd.h>

using namespace Lingmo;

namespace dm {

// The escapes the Desktop Entry specification defines for string values.
static std::string unescape(const std::string &value)
{
    std::string result;
    result.reserve(value.size());

    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c != '\\' || i + 1 == value.size()) {
            result += c;
            continue;
        }

        const char next = value[++i];
        switch (next) {
        case 's': result += ' '; break;
        case 'n': result += '\n'; break;
        case 't': result += '\t'; break;
        case 'r': result += '\r'; break;
        default: result += next; break;
        }
    }

    return result;
}

static bool isTrue(const std::string &value)
{
    return base::strings::trimmed(value) == "true";
}

static bool executableExists(const std::string &program)
{
    if (base::strings::startsWith(program, "/"))
        return access(program.c_str(), X_OK) == 0;

    return !base::paths::findExecutable(program).empty();
}

static std::string value(const std::map<std::string, std::string> &keys, const std::string &key,
                         const std::string &fallback = {})
{
    const auto it = keys.find(key);
    return it == keys.end() ? fallback : it->second;
}

std::optional<SessionEntry> readSessionFile(const std::string &path)
{
    const std::optional<std::string> contents = base::file::read(path);
    if (!contents)
        return std::nullopt;

    const std::string text = base::strings::sanitizedUtf8(*contents);
    std::map<std::string, std::string> keys;
    bool inEntry = false;
    for (const std::string_view raw : base::strings::splitViews(text, '\n')) {
        const std::string line(base::strings::trimmed(raw));
        if (line.empty() || line.front() == '#')
            continue;

        if (line.front() == '[') {
            inEntry = line == "[Desktop Entry]";
            continue;
        }

        const size_t equals = line.find('=');
        if (!inEntry || equals == 0 || equals == std::string::npos)
            continue;

        const std::string key(base::strings::trimmed(std::string_view(line).substr(0, equals)));
        keys.emplace(key, std::string(base::strings::trimmed(std::string_view(line).substr(equals + 1))));
    }

    if (isTrue(value(keys, "Hidden")) || isTrue(value(keys, "NoDisplay")))
        return std::nullopt;

    const std::string tryExec = unescape(value(keys, "TryExec"));
    if (!tryExec.empty() && !executableExists(tryExec))
        return std::nullopt;

    SessionEntry entry;
    const std::string fileName = path.substr(path.rfind('/') + 1);
    entry.id = fileName.substr(0, fileName.rfind('.'));
    entry.name = unescape(value(keys, "Name", entry.id));

    // Field codes have no meaning for a session and are dropped.
    for (const std::string &argument : splitCommand(unescape(value(keys, "Exec")))) {
        if (!(argument.size() == 2 && argument.front() == '%'))
            entry.exec.push_back(argument);
    }

    for (const std::string &name : base::strings::split(value(keys, "DesktopNames"), ';')) {
        const std::string trimmed(base::strings::trimmed(name));
        if (!trimmed.empty())
            entry.desktopNames.push_back(unescape(trimmed));
    }

    if (entry.exec.empty())
        return std::nullopt;

    return entry;
}

// The visible regular *.desktop files of a directory, by name.
static std::vector<std::string> desktopFiles(const std::string &directory)
{
    std::vector<std::string> names;
    DIR *entries = opendir(directory.c_str());
    if (!entries)
        return names;
    while (const dirent *entry = readdir(entries)) {
        const std::string name = entry->d_name;
        if (name.front() == '.' || !base::strings::endsWith(name, ".desktop"))
            continue;
        struct stat info = {};
        if (stat((directory + "/" + name).c_str(), &info) == 0 && S_ISREG(info.st_mode))
            names.push_back(name);
    }
    closedir(entries);
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<SessionEntry> findSessions(const std::vector<std::string> &directories)
{
    std::vector<SessionEntry> sessions;
    std::set<std::string> seen;

    for (const std::string &directory : directories) {
        for (const std::string &name : desktopFiles(directory)) {
            const std::string id = name.substr(0, name.size() - std::string_view(".desktop").size());
            if (seen.count(id))
                continue;

            // A hidden entry masks the same id further down, as with autostart.
            seen.insert(id);
            if (auto entry = readSessionFile(directory + "/" + name))
                sessions.push_back(*entry);
        }
    }

    return sessions;
}

std::optional<SessionEntry> findSession(const std::vector<std::string> &directories, const std::string &id)
{
    // An id comes from the greeter; it must name a file, not a path.
    if (id.empty() || id.find('/') != std::string::npos || id.front() == '.')
        return std::nullopt;

    for (const SessionEntry &session : findSessions(directories)) {
        if (session.id == id)
            return session;
    }

    return std::nullopt;
}

std::vector<std::string> sessionEnvironment(const SessionEntry &session, int vt)
{
    std::vector<std::string> environment {
            "XDG_SESSION_TYPE=wayland",
            "XDG_SESSION_CLASS=user",
            "XDG_SEAT=seat0",
            "XDG_VTNR=" + std::to_string(vt),
    };

    // A bare command, as a greetd greeter sends, names no session.
    if (!session.id.empty()) {
        environment.push_back("DESKTOP_SESSION=" + session.id);
        environment.push_back("XDG_SESSION_DESKTOP="
                              + (session.desktopNames.empty() ? session.id : session.desktopNames.front()));
    }

    if (!session.desktopNames.empty())
        environment.push_back("XDG_CURRENT_DESKTOP=" + base::strings::join(session.desktopNames, ":"));

    return environment;
}

std::vector<std::string> greeterEnvironment(int vt, const std::string &socket)
{
    return {
            "XDG_SESSION_TYPE=wayland",
            "XDG_SESSION_CLASS=greeter",
            "XDG_SEAT=seat0",
            "XDG_VTNR=" + std::to_string(vt),
            "GREETD_SOCK=" + socket,
    };
}

} // namespace dm

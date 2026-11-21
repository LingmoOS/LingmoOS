// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/launcher.h"
#include "core/common.h"
#include "core/standardfolders.h"

#include "base/application_registry.h"
#include "base/dbus.h"
#include "base/file.h"
#include "base/mime_apps.h"
#include "base/mime_database.h"
#include "base/process.h"

#include <algorithm>
#include <unistd.h>
#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>

namespace fm::launcher {

static constexpr char kRuntime[] = "org.lingmo.AppRuntime";
static constexpr char kRuntimePath[] = "/AppRuntime";

static base::dbus::Proxy runtime()
{
    base::dbus::Proxy proxy(base::dbus::Connection::session(), kRuntime, kRuntimePath, kRuntime);
    // A runtime that stops answering must not freeze the window for half a minute.
    proxy.setTimeout(std::chrono::seconds(5));
    return proxy;
}

// Hidden entries only mask a system entry; programs that need a terminal
// are not offered for opening files.
static bool opensFiles(const base::DesktopEntry& entry)
{
    return !entry.hidden && !entry.terminal;
}

std::string defaultApplication(const std::string& mimeType)
{
    return base::mimeapps::chosenApplication(base::mimeapps::listPaths(), mimeType);
}

std::vector<std::string> recommendedApplications(const std::string& mimeType)
{
    return base::mimeapps::applications(mimeType, base::mimeapps::listPaths());
}

std::vector<Application> applicationsFor(const std::string&, const std::string& mimeType)
{
    std::vector<std::string> applications = recommendedApplications(mimeType);
    std::string chosen = defaultApplication(mimeType);
    // As Open does: without a choice, the first recommended application.
    if (chosen.empty() && !applications.empty())
        chosen = applications.front();
    if (!chosen.empty()) {
        std::erase(applications, chosen);
        applications.insert(applications.begin(), chosen);
    }

    std::vector<Application> result;
    auto& registry = base::ApplicationRegistry::instance();
    for (const std::string& path : applications) {
        const auto entry = registry.byPath(path);
        if (!entry)
            continue;
        result.push_back({path, entry->name, entry->icon});
    }
    return result;
}

bool setDefaultApplication(const std::string& mimeType, const std::string& desktopFile)
{
    const auto entry = base::ApplicationRegistry::instance().byPath(desktopFile);
    if (!entry)
        return false;
    return base::mimeapps::setDefault(base::mimeapps::userListPath(), mimeType, entry->id + ".desktop");
}

static std::function<void(std::function<void(const std::string&)>)> s_tokenSource;

static bool startCommandWithToken(const std::vector<std::string>& command, const std::string& workingDirectory,
                                  const std::string& appId, const std::string& token)
{
    const auto reply = runtime().call("launchCommand", appId, command, workingDirectory, token);
    if (reply.value<uint32_t>().value_or(0) != 0)
        return true;

    std::vector<std::string> arguments(command.begin() + 1, command.end());
    return base::Process::startDetached(command.front(), arguments, workingDirectory) > 0;
}

bool startCommand(const std::vector<std::string>& command, const std::string& workingDirectory,
                  const std::string& appId)
{
    if (command.empty() || command.front().empty())
        return false;
    if (s_tokenSource) {
        s_tokenSource([command, workingDirectory, appId](const std::string& token) {
            startCommandWithToken(command, workingDirectory, appId, token);
        });
        return true;
    }
    return startCommandWithToken(command, workingDirectory, appId, std::string());
}

bool keepFromChildren(int fd)
{
    const int flags = ::fcntl(fd, F_GETFD);
    return flags >= 0 && ::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == 0;
}

void setActivationTokenSource(std::function<void(std::function<void(const std::string& token)>)> source)
{
    s_tokenSource = std::move(source);
}

static bool launchWithToken(const std::string& desktopFile, const std::vector<std::string>& arguments,
                            const std::string& token)
{
    // The runtime knows the entry: it expands Exec and tracks the instance
    // under the application's own id.
    const auto reply = runtime().call("launchApplication", desktopFile, arguments, token);
    if (reply.value<uint32_t>().value_or(0) != 0)
        return true;

    // Without the runtime, or for an entry it does not know, Exec is expanded here.
    const auto contents = base::file::read(desktopFile);
    if (!contents)
        return false;
    std::string id = fileName(desktopFile);
    if (id.ends_with(".desktop"))
        id.resize(id.size() - 8);
    const auto entry = base::DesktopEntry::parse(id, desktopFile, *contents);
    if (!entry)
        return false;
    return startCommandWithToken(entry->commandForArguments(arguments), entry->workingDirectory, entry->id, token);
}

bool launchApplication(const std::string& desktopFile, const std::string& file)
{
    std::vector<std::string> arguments;
    if (!file.empty())
        arguments.push_back(file);
    // The new window takes the focus with a token from the window the user is in.
    if (s_tokenSource) {
        if (desktopFile.find('/') != std::string::npos && ::access(desktopFile.c_str(), R_OK) != 0)
            return false;
        s_tokenSource([desktopFile, arguments](const std::string& token) {
            launchWithToken(desktopFile, arguments, token);
        });
        return true;
    }
    return launchWithToken(desktopFile, arguments, std::string());
}

static bool isDesktopFile(const std::string& path)
{
    return base::mime::inherits(base::mime::typeForFile(path), "application/x-desktop");
}

bool launchFile(const std::string& path)
{
    if (isDesktopFile(path))
        return launchApplication(path);
    return startCommand({path});
}

bool isTrusted(const std::string& path)
{
    if (!isDesktopFile(path))
        return false;

    // A local launcher the user made executable is trusted; the first run of
    // a launcher that is not asks, and sets this bit.
    struct stat st;
    if (::stat(path.c_str(), &st) == 0 && (st.st_mode & S_IXUSR))
        return true;

    char resolved[PATH_MAX];
    if (!::realpath(parentPath(path).c_str(), resolved))
        return false;
    const std::string directory = resolved;
    for (const std::string& location : base::ApplicationRegistry::desktopPaths()) {
        char canonical[PATH_MAX];
        if (!::realpath(location.c_str(), canonical))
            continue;
        const std::string base = canonical;
        if (directory == base || directory.starts_with(base + '/'))
            return true;
    }
    return false;
}

bool trustAndLaunch(const std::string& path)
{
    struct stat st;
    if (::stat(path.c_str(), &st) != 0)
        return false;
    // Fails on mounts without permissions; the user agreed to run it all the same.
    if (!(st.st_mode & S_IXUSR))
        ::chmod(path.c_str(), st.st_mode | S_IXUSR);
    return launchFile(path);
}

void launchTerminal(const std::string& folder)
{
    auto& registry = base::ApplicationRegistry::instance();
    const auto isTerminal = [](const base::DesktopEntry& entry) {
        return opensFiles(entry)
                && std::find(entry.categories.begin(), entry.categories.end(), "TerminalEmulator")
                != entry.categories.end();
    };

    std::string preferred = base::mimeapps::defaultTerminal(base::mimeapps::userTerminalsListPath());
    if (preferred.ends_with(".desktop"))
        preferred.resize(preferred.size() - 8);
    std::shared_ptr<const base::DesktopEntry> terminal = preferred.empty() ? nullptr : registry.byId(preferred);
    if (!terminal || !isTerminal(*terminal)) {
        terminal = nullptr;
        for (const auto& entry : registry.entries()) {
            if (isTerminal(*entry)) {
                terminal = entry;
                break;
            }
        }
    }
    if (terminal)
        startCommand(terminal->commandForArguments({}), folder, terminal->id);
}

} // namespace fm::launcher

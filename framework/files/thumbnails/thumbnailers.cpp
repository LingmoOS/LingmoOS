// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailers_p.h"

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <set>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include "base/elapsed_timer.h"
#include "base/file.h"
#include "base/paths.h"
#include "base/strings.h"
#include "lingmo/files/mime_types.h"
#include "lingmo/files/url.h"
#include "../mime_sniffer_p.h"
#include "../path_utils_p.h"

extern char** environ;

namespace Lingmo::Files::internal {

using namespace Lingmo::Base;

// Decoders of untrusted media can hang; a stuck one must not hold a pool thread for good.
static constexpr int timeoutMs = 10000;
static constexpr int64_t refreshIntervalMs = 3000;

static std::string canonical(const std::string& mimeType)
{
    const std::string name = mime::canonicalName(mimeType);
    return name.empty() ? mimeType : name;
}

static std::vector<std::string> thumbnailerDirectories()
{
    std::vector<std::string> directories;
    std::vector<std::string> data{userDataDirectory()};
    for (const std::string& directory : paths::dataDirectories())
        data.push_back(directory);

    for (const std::string& directory : data)
        directories.push_back(directory + "/thumbnailers");

    return directories;
}

static bool isExecutable(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode) && ::access(path.c_str(), X_OK) == 0;
}

// The program's path, or empty when it cannot run.
static std::string resolve(const std::string& program)
{
    if (program.empty())
        return {};

    if (program.starts_with('/'))
        return isExecutable(program) ? program : std::string();

    return paths::findExecutable(program);
}

// A command line's words, split at spaces; double quotes keep a phrase
// together and three of them stand for one literal quote.
static std::vector<std::string> splitCommand(std::string_view command)
{
    std::vector<std::string> arguments;
    std::string current;
    bool inQuote = false;
    bool any = false;
    int quotes = 0;

    for (size_t i = 0; i < command.size(); ++i) {
        const char c = command[i];
        if (c == '"') {
            ++quotes;
            if (quotes == 3) {
                quotes = 0;
                current += '"';
            }
            continue;
        }

        if (quotes) {
            if (quotes == 1)
                inQuote = !inQuote;
            quotes = 0;
        }

        if (!inQuote && strings::isAsciiWhitespace(c)) {
            if (!current.empty() || any)
                arguments.push_back(current);
            current.clear();
            any = false;
        } else {
            current += c;
            any = true;
        }
    }

    if (quotes == 1)
        inQuote = !inQuote;
    if (!current.empty() || any)
        arguments.push_back(current);

    return arguments;
}

namespace {

struct Entry
{
    std::vector<std::string> command;
    std::vector<std::string> mimeTypes;
};

} // namespace

// The key file subset thumbnailers use: one group, plain keys, no localized values.
static Entry parseEntry(const std::string& path)
{
    const std::optional<std::string> contents = file::read(path);
    if (!contents)
        return {};

    bool inGroup = false;
    std::string exec;
    std::string tryExec;
    std::string mimeTypes;
    for (const std::string_view rawLine : strings::splitViews(*contents, '\n')) {
        const std::string_view line = strings::trimmed(rawLine);
        if (line.empty() || line.starts_with('#'))
            continue;

        if (line.starts_with('[')) {
            inGroup = line == "[Thumbnailer Entry]";
            continue;
        }

        const size_t equals = line.find('=');
        if (!inGroup || equals == std::string_view::npos || equals == 0)
            continue;

        const std::string_view key = strings::trimmed(line.substr(0, equals));
        const std::string value(strings::trimmed(line.substr(equals + 1)));
        if (key == "Exec")
            exec = value;
        else if (key == "TryExec")
            tryExec = value;
        else if (key == "MimeType")
            mimeTypes = value;
    }

    const std::vector<std::string> command = splitCommand(exec);
    if (command.empty() || resolve(command.front()).empty() || (!tryExec.empty() && resolve(tryExec).empty()))
        return {};

    return {command, strings::split(mimeTypes, ';', strings::SplitBehavior::SkipEmptyParts)};
}

static std::vector<std::string> expand(const std::vector<std::string>& command, const std::string& input,
                                       const std::string& output, int size)
{
    std::vector<std::string> arguments;
    for (const std::string& argument : command) {
        std::string expanded;
        for (size_t i = 0; i < argument.size(); ++i) {
            if (argument[i] != '%' || i + 1 == argument.size()) {
                expanded += argument[i];
                continue;
            }

            const char code = argument[++i];
            if (code == 'i')
                expanded += input;
            else if (code == 'u')
                expanded += Url::fromLocalFile(input).toString();
            else if (code == 'o')
                expanded += output;
            else if (code == 's')
                expanded += strings::number(size);
            else if (code == '%')
                expanded += '%';
        }
        arguments.push_back(expanded);
    }

    return arguments;
}

Thumbnailers* Thumbnailers::self()
{
    static Thumbnailers* thumbnailers = new Thumbnailers;
    return thumbnailers;
}

bool Thumbnailers::contains(const std::string& mimeType)
{
    return !command(mimeType).empty();
}

std::vector<std::string> Thumbnailers::command(const std::string& mimeType)
{
    const std::string name = canonical(mimeType);

    const std::lock_guard lock(m_mutex);
    if (!m_checked.isValid() || m_checked.elapsedMs() >= refreshIntervalMs)
        refreshLocked();

    const auto found = m_commands.find(name);
    return found == m_commands.cend() ? std::vector<std::string>() : found->second;
}

static int64_t modifiedMs(const std::string& path)
{
    struct stat st;
    if (::stat(path.c_str(), &st) != 0)
        return -1;
    return int64_t(st.st_mtim.tv_sec) * 1000 + st.st_mtim.tv_nsec / 1000000;
}

static std::vector<std::string> thumbnailerFiles(const std::string& directory)
{
    std::vector<std::string> result;
    DIR* dir = ::opendir(directory.c_str());
    if (!dir)
        return result;

    while (const dirent* entry = ::readdir(dir)) {
        const std::string name = entry->d_name;
        struct stat st;
        if (name.ends_with(".thumbnailer") && ::stat((directory + '/' + name).c_str(), &st) == 0
            && S_ISREG(st.st_mode))
            result.push_back(name);
    }
    ::closedir(dir);

    std::sort(result.begin(), result.end());
    return result;
}

void Thumbnailers::refreshLocked()
{
    m_checked.start();

    std::vector<std::pair<std::string, int64_t>> directories;
    for (const std::string& directory : thumbnailerDirectories())
        directories.emplace_back(directory, modifiedMs(directory));

    // Package installs add or remove files, which touches the directory.
    if (directories == m_directories)
        return;

    m_directories = directories;
    m_commands.clear();

    std::set<std::string> seen;
    for (const auto& [directory, modified] : m_directories) {
        for (const std::string& name : thumbnailerFiles(directory)) {
            // A file in an earlier directory hides the one of the same name after it.
            if (!seen.insert(name).second)
                continue;

            const Entry entry = parseEntry(directory + '/' + name);
            for (const std::string& mimeType : entry.mimeTypes)
                m_commands.emplace(canonical(mimeType), entry.command);
        }
    }
}

// Waits for the child at most timeout milliseconds; false when it still runs.
static bool waitFor(pid_t pid, int timeout, int* status)
{
    const int pidfd = int(::syscall(SYS_pidfd_open, pid, 0));
    if (pidfd >= 0) {
        pollfd descriptor{pidfd, POLLIN, 0};
        int result;
        do {
            result = ::poll(&descriptor, 1, timeout);
        } while (result < 0 && errno == EINTR);
        ::close(pidfd);
        if (result == 0)
            return false;
    } else {
        ElapsedTimer timer;
        timer.start();
        while (::waitpid(pid, status, WNOHANG) == 0) {
            if (timer.elapsedMs() >= timeout)
                return false;
            ::usleep(10000);
        }
        return true;
    }

    while (::waitpid(pid, status, 0) < 0 && errno == EINTR) {
    }
    return true;
}

ui::Bitmap Thumbnailers::generate(const std::string& path, const std::string& mimeType, base::Size size)
{
    const std::vector<std::string> command = this->command(mimeType);
    if (command.empty())
        return {};

    const file::TemporaryDirectory directory("/tmp", "lingmo-thumbnail-");
    if (!directory.isValid())
        return {};

    const std::string output = directory.path() + "/thumbnail.png";
    std::vector<std::string> arguments =
        expand(command, absoluteFilePath(path), output, std::max(size.width(), size.height()));
    const std::string program = resolve(arguments.front());
    if (program.empty())
        return {};

    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attributes;
    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_init(&attributes);
    posix_spawn_file_actions_addopen(&actions, 0, "/dev/null", O_RDONLY, 0);
    posix_spawn_file_actions_addopen(&actions, 1, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, 2, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addchdir_np(&actions, directory.path().c_str());
    // Its own process group, so a timeout also stops what a wrapper script started.
    posix_spawnattr_setpgroup(&attributes, 0);
    sigset_t defaults;
    sigemptyset(&defaults);
    sigaddset(&defaults, SIGPIPE);
    posix_spawnattr_setsigdefault(&attributes, &defaults);
    sigset_t none;
    sigemptyset(&none);
    posix_spawnattr_setsigmask(&attributes, &none);
    posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSIGMASK);

    std::vector<char*> argv;
    for (std::string& argument : arguments)
        argv.push_back(argument.data());
    argv.push_back(nullptr);

    pid_t pid = -1;
    const int spawned = ::posix_spawn(&pid, program.c_str(), &actions, &attributes, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attributes);
    if (spawned != 0)
        return {};

    int status = 0;
    if (!waitFor(pid, timeoutMs, &status)) {
        ::kill(-pid, SIGKILL);
        waitFor(pid, -1, &status);
        return {};
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return {};

    ui::Bitmap image = ui::Bitmap::decodeFile(output);
    if (image.isNull())
        return {};

    // Thumbnailers may write a larger image than asked for.
    if (image.width() > size.width() || image.height() > size.height())
        image = image.scaled(ui::scaledSize(image.size(), size, ui::Bitmap::Scaling::Fit));

    return image;
}

} // namespace Lingmo::Files::internal

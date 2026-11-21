// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/accounts/passwordchanger.h"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "base/event_loop.h"
#include "base/paths.h"

using namespace Lingmo;

namespace settings {

// Through syscall(): glibc 2.41's sys/pidfd.h lacks the C linkage C++ needs.
static int openPidfd(pid_t pid)
{
    return int(syscall(SYS_pidfd_open, pid, 0));
}

static void closeFd(int& fd)
{
    if (fd >= 0)
        ::close(fd);
    fd = -1;
}

static void wipe(std::string& secret)
{
    explicit_bzero(secret.data(), secret.size());
    secret.clear();
}

static std::string trimmed(const std::string& text)
{
    const size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// What passwd said, without its prompts and its closing "password unchanged".
static std::string passwdMessage(const std::string& output)
{
    std::string message;
    size_t start = 0;
    while (start < output.size()) {
        size_t end = output.find('\n', start);
        if (end == std::string::npos)
            end = output.size();
        std::string line = trimmed(output.substr(start, end - start));
        start = end + 1;
        if (line.starts_with("passwd: "))
            line.erase(0, 8);
        if (line.empty() || line.ends_with("password:") || line == "password unchanged"
            || line.starts_with("Changing password for "))
            continue;
        if (!message.empty())
            message += ' ';
        message += line;
    }
    return message;
}

PasswordChanger::PasswordChanger(std::string program, std::chrono::milliseconds timeout, base::EventLoop* loop)
    : m_loop(loop ? *loop : *base::EventLoop::current())
    , m_program(std::move(program))
    , m_timeout(timeout)
    , m_silence(&m_loop)
{
    m_silence.setSingleShot(true);
    m_silence.onTimeout = [this] { stop(Result::TimedOut, {}, SIGKILL); };
}

PasswordChanger::~PasswordChanger()
{
    if (m_pid > 0) {
        // A passwd writing the password files finishes that first.
        ::kill(m_pid, SIGTERM);
        pollfd ended = {m_pidfd, POLLIN, 0};
        if (poll(&ended, 1, 2000) <= 0)
            ::kill(m_pid, SIGKILL);
        siginfo_t info{};
        waitid(P_PIDFD, id_t(m_pidfd), &info, WEXITED);
        m_pid = -1;
    }
    closeAll();
}

bool PasswordChanger::start(std::string currentPassword, std::string newPassword)
{
    if (m_pid > 0)
        return false;
    m_current = std::move(currentPassword);
    m_new = std::move(newPassword);
    m_currentAnswered = false;
    m_newAnswered = false;
    m_stopping = false;

    m_master = posix_openpt(O_RDWR | O_NOCTTY | O_CLOEXEC | O_NONBLOCK);
    char name[64] = {};
    if (m_master < 0 || grantpt(m_master) != 0 || unlockpt(m_master) != 0
        || ptsname_r(m_master, name, sizeof(name)) != 0) {
        closeAll();
        return false;
    }
    // Held until passwd has the terminal, so that it never hangs up before.
    int terminal = ::open(name, O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (terminal < 0) {
        closeAll();
        return false;
    }

    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attributes;
    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_init(&attributes);
    // In a session of its own, the terminal opened after setsid() becomes
    // passwd's controlling terminal, where PAM turns echo off.
    posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETSID | POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF);
    sigset_t none;
    sigemptyset(&none);
    posix_spawnattr_setsigmask(&attributes, &none);
    sigset_t defaults;
    sigfillset(&defaults);
    posix_spawnattr_setsigdefault(&attributes, &defaults);
    posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, name, O_RDWR, 0);
    posix_spawn_file_actions_adddup2(&actions, STDIN_FILENO, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, STDIN_FILENO, STDERR_FILENO);
    posix_spawn_file_actions_addclosefrom_np(&actions, STDERR_FILENO + 1);

    // English prompts and messages to match. The keyring's password module
    // finds its daemon under the runtime directory.
    std::vector<std::string> environment = {"LC_ALL=C", "PATH=/usr/sbin:/usr/bin:/sbin:/bin"};
    const std::string runtime = base::paths::runtimeDirectory();
    if (!runtime.empty())
        environment.push_back("XDG_RUNTIME_DIR=" + runtime);
    std::vector<char*> envp;
    for (std::string& entry : environment)
        envp.push_back(entry.data());
    envp.push_back(nullptr);
    char* argv[] = {m_program.data(), nullptr};

    pid_t pid = -1;
    const int spawned = posix_spawn(&pid, m_program.c_str(), &actions, &attributes, argv, envp.data());
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attributes);
    ::close(terminal);
    if (spawned != 0) {
        closeAll();
        return false;
    }
    m_pid = pid;
    m_pidfd = openPidfd(pid);
    if (m_pidfd < 0) {
        ::kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        m_pid = -1;
        closeAll();
        return false;
    }

    m_masterWatch = m_loop.watch(m_master, EPOLLIN, [this](uint32_t) { readOutput(); });
    m_pidWatch = m_loop.watch(m_pidfd, EPOLLIN, [this](uint32_t) { processEnded(); });
    m_silence.start(m_timeout);
    return true;
}

void PasswordChanger::drain()
{
    char buffer[4096];
    for (;;) {
        const ssize_t count = ::read(m_master, buffer, sizeof(buffer));
        if (count > 0) {
            m_output.append(buffer, size_t(count));
            if (!m_stopping)
                m_silence.start(m_timeout);
            continue;
        }
        if (count < 0 && errno == EINTR)
            continue;
        // EIO once passwd and everything it started have closed the terminal.
        if (count == 0 || errno != EAGAIN) {
            if (m_masterWatch)
                m_loop.unwatch(m_masterWatch);
            m_masterWatch = 0;
        }
        break;
    }
}

void PasswordChanger::readOutput()
{
    drain();
    const std::string text = trimmed(m_output);
    if (text.ends_with("Retype new password:"))
        handlePrompt(Prompt::Retype);
    else if (text.ends_with("New password:"))
        handlePrompt(Prompt::New);
    // The second is pam_unix's before Linux-PAM 1.4.
    else if (text.ends_with("Current password:") || text.ends_with("(current) UNIX password:"))
        handlePrompt(Prompt::Current);
}

void PasswordChanger::handlePrompt(Prompt prompt)
{
    if (m_stopping)
        return;
    switch (prompt) {
    case Prompt::Current:
        if (m_currentAnswered) {
            stop(Result::WrongPassword, {}, SIGTERM);
            return;
        }
        m_currentAnswered = true;
        answer(m_current);
        break;
    case Prompt::New:
        // Asked again: the last one was refused, and passwd said why.
        if (m_newAnswered) {
            stop(Result::Rejected, passwdMessage(m_output), SIGTERM);
            return;
        }
        m_newAnswered = true;
        answer(m_new);
        break;
    case Prompt::Retype:
        answer(m_new);
        break;
    }
}

void PasswordChanger::answer(const std::string& text)
{
    m_output.clear();
    std::string line = text + '\n';
    size_t written = 0;
    while (written < line.size()) {
        const ssize_t count = ::write(m_master, line.data() + written, line.size() - written);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            break;
        written += size_t(count);
    }
    const bool complete = written == line.size();
    wipe(line);
    if (!complete)
        stop(Result::Failed, {}, SIGTERM);
}

void PasswordChanger::processEnded()
{
    if (m_masterWatch)
        drain();
    siginfo_t info{};
    if (waitid(P_PIDFD, id_t(m_pidfd), &info, WEXITED | WNOHANG) != 0 || info.si_pid == 0)
        return;
    m_pid = -1;
    const bool exited = info.si_code == CLD_EXITED;
    const std::string message = passwdMessage(m_output);

    Result result;
    std::string said;
    if (m_stopping) {
        result = m_stopResult;
        said = m_stopMessage;
    } else if (exited && info.si_status == 0) {
        result = Result::Changed;
    } else if (!m_newAnswered
               && (m_output.find("Authentication token manipulation error") != std::string::npos
                   || m_output.find("Authentication failure") != std::string::npos)) {
        result = Result::WrongPassword;
    } else {
        result = Result::Failed;
        said = message;
    }
    closeAll();

    const auto finished = onFinished;
    if (finished)
        finished(result, said);
}

void PasswordChanger::stop(Result result, std::string message, int signal)
{
    if (m_stopping || m_pid <= 0)
        return;
    m_stopping = true;
    m_stopResult = result;
    m_stopMessage = std::move(message);
    m_silence.stop();
    ::kill(m_pid, signal);
}

void PasswordChanger::closeAll()
{
    m_silence.stop();
    if (m_masterWatch)
        m_loop.unwatch(m_masterWatch);
    if (m_pidWatch)
        m_loop.unwatch(m_pidWatch);
    m_masterWatch = 0;
    m_pidWatch = 0;
    closeFd(m_master);
    closeFd(m_pidfd);
    wipe(m_current);
    wipe(m_new);
    wipe(m_output);
}

} // namespace settings

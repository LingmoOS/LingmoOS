// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/detached_process.h"

#include "base/event_loop.h"

#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

namespace Lingmo::Core {

void dropOomProtection(const char* path)
{
    const int input = ::open(path, O_RDONLY | O_CLOEXEC);
    if (input < 0)
        return;
    char value[16];
    const ssize_t length = ::read(input, value, sizeof(value));
    ::close(input);
    if (length <= 0 || value[0] != '-')
        return;

    const int output = ::open(path, O_WRONLY | O_TRUNC | O_CLOEXEC);
    if (output < 0)
        return;
    if (::write(output, "0", 1) != 1) {
    }
    ::close(output);
}

pid_t startDetached(const std::vector<std::string>& command, const std::string& workingDirectory,
                    const Environment& environment)
{
    if (command.empty() || command.front().empty())
        return -1;

    std::vector<char*> argv;
    for (const std::string& argument : command)
        argv.push_back(const_cast<char*>(argument.c_str()));
    argv.push_back(nullptr);
    const std::vector<std::string> entries = environmentEntries(environment);
    std::vector<char*> envp;
    for (const std::string& entry : entries)
        envp.push_back(const_cast<char*>(entry.c_str()));
    envp.push_back(nullptr);

    // Carries the errno of a failed start; a successful exec closes it.
    int failure[2];
    if (pipe2(failure, O_CLOEXEC) != 0)
        return -1;

    const pid_t pid = fork();
    if (pid == 0) {
        ::close(failure[0]);

        // Only async-signal-safe calls from here: this process may have threads.
        // The program starts with default handlers and no signal blocked,
        // whatever this process changed.
        struct sigaction defaults = {};
        defaults.sa_handler = SIG_DFL;
        for (int signal = 1; signal < NSIG; ++signal)
            sigaction(signal, &defaults, nullptr);
        sigset_t none;
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, nullptr);
        setsid();
        dropOomProtection("/proc/self/oom_score_adj");

        if (workingDirectory.empty() || chdir(workingDirectory.c_str()) == 0)
            execvpe(argv.front(), argv.data(), envp.data());
        const int error = errno;
        if (::write(failure[1], &error, sizeof(error)) != sizeof(error)) {
        }
        _exit(127);
    }
    ::close(failure[1]);
    if (pid < 0) {
        ::close(failure[0]);
        return -1;
    }

    int error = 0;
    ssize_t length;
    while ((length = ::read(failure[0], &error, sizeof(error))) < 0 && errno == EINTR) {
    }
    ::close(failure[0]);
    if (length > 0) {
        while (waitpid(pid, nullptr, 0) < 0 && errno == EINTR) {
        }
        return -1;
    }

    // Reaped when it ends, so it never lingers as a zombie; after this
    // process, init does it.
    const int pidfd = int(syscall(SYS_pidfd_open, pid, 0));
    base::EventLoop* loop = base::EventLoop::current();
    if (pidfd >= 0 && loop) {
        auto watch = std::make_shared<int>(0);
        *watch = loop->watch(pidfd, EPOLLIN, [loop, pidfd, watch](uint32_t) {
            siginfo_t info{};
            if (waitid(P_PIDFD, id_t(pidfd), &info, WEXITED | WNOHANG) != 0 || info.si_pid == 0)
                return;
            loop->unwatch(*watch);
            ::close(pidfd);
        });
    } else if (pidfd >= 0) {
        ::close(pidfd);
    }
    return pid;
}

} // namespace Lingmo::Core

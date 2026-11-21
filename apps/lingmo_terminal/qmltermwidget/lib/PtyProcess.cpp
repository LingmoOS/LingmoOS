// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PtyProcess.h"

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <linux/close_range.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/paths.h"

#include "tools.h"

extern char** environ;

namespace Konsole
{

PtyProcess::PtyProcess()
    : _loop(*Lingmo::base::EventLoop::current())
{
    _master = ::posix_openpt(O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (_master < 0) {
        LM_WARNING(qtermwidgetLogger) << "Can't open a pseudo teletype";
        return;
    }
    if (grantpt(_master) != 0 || unlockpt(_master) != 0) {
        LM_WARNING(qtermwidgetLogger) << "Can't open a pseudo teletype";
        ::close(_master);
        _master = -1;
        return;
    }
    const char* name = ptsname(_master);
    _slave = name ? ::open(name, O_RDWR | O_NOCTTY | O_CLOEXEC) : -1;
    if (_slave < 0) {
        LM_WARNING(qtermwidgetLogger) << "Can't open slave pseudo teletype";
        ::close(_master);
        _master = -1;
        return;
    }
    fcntl(_master, F_SETFL, fcntl(_master, F_GETFL) | O_NONBLOCK);

    _masterWatch = _loop.watch(_master, EPOLLIN, [this](uint32_t events) {
        if (events & EPOLLOUT)
            writePending();
        if (events & (EPOLLIN | EPOLLHUP | EPOLLERR))
            readMaster();
    });
}

PtyProcess::~PtyProcess()
{
    if (_pidWatch)
        _loop.unwatch(_pidWatch);
    if (_pidFd >= 0)
        ::close(_pidFd);
    closePty();
}

void PtyProcess::closePty()
{
    if (_masterWatch)
        _loop.unwatch(_masterWatch);
    _masterWatch = 0;
    if (_master >= 0)
        ::close(_master);
    if (_slave >= 0)
        ::close(_slave);
    _master = -1;
    _slave = -1;
}

void PtyProcess::applyAttributes()
{
    if (_slave < 0)
        return;

    struct ::termios ttmode;
    if (tcgetattr(_slave, &ttmode) != 0)
        return;

    if (!_xonXoff)
        ttmode.c_iflag &= ~(IXOFF | IXON);
    else
        ttmode.c_iflag |= (IXOFF | IXON);

    if (!_utf8)
        ttmode.c_iflag &= ~IUTF8;
    else
        ttmode.c_iflag |= IUTF8;

    if (_eraseChar != 0)
        ttmode.c_cc[VERASE] = _eraseChar;

    if (tcsetattr(_slave, TCSANOW, &ttmode) != 0)
        LM_WARNING(qtermwidgetLogger) << "Unable to set terminal attributes.";
}

void PtyProcess::setWindowSize(int lines, int columns)
{
    _windowColumns = columns;
    _windowLines = lines;
    if (_master >= 0) {
        struct winsize size = {};
        size.ws_row = (unsigned short)lines;
        size.ws_col = (unsigned short)columns;
        ioctl(_master, TIOCSWINSZ, &size);
    }
}

Lingmo::base::Size PtyProcess::windowSize() const
{
    return Lingmo::base::Size(_windowColumns, _windowLines);
}

void PtyProcess::setFlowControlEnabled(bool enable)
{
    _xonXoff = enable;
    applyAttributes();
}

bool PtyProcess::flowControlEnabled() const
{
    if (_slave >= 0) {
        struct ::termios ttmode;
        if (tcgetattr(_slave, &ttmode) == 0)
            return (ttmode.c_iflag & IXOFF) && (ttmode.c_iflag & IXON);
    }
    LM_WARNING(qtermwidgetLogger) << "Unable to get flow control status, terminal not connected.";
    return false;
}

void PtyProcess::setUtf8Mode(bool enable)
{
    _utf8 = enable;
    applyAttributes();
}

void PtyProcess::setErase(char erase)
{
    _eraseChar = erase;
    applyAttributes();
}

char PtyProcess::erase() const
{
    if (_slave >= 0) {
        struct ::termios ttyAttributes;
        if (tcgetattr(_slave, &ttyAttributes) == 0)
            return char(ttyAttributes.c_cc[VERASE]);
    }
    return _eraseChar;
}

void PtyProcess::setWorkingDirectory(const std::string& directory)
{
    _workingDirectory = directory;
}

static std::string variableName(const std::string& binding)
{
    return binding.substr(0, binding.find('='));
}

bool PtyProcess::start(const std::string& program, const std::vector<std::string>& arguments,
                       const std::vector<std::string>& environment, const std::vector<std::string>& unset,
                       const std::vector<std::string>& defaults)
{
    if (_master < 0 || _pid > 0)
        return false;

    applyAttributes();
    setWindowSize(_windowLines, _windowColumns);

    // The environment and argv are built before the fork: only async-signal-safe
    // calls may follow it.
    std::vector<std::string> bindings;
    for (char** variable = environ; *variable; ++variable) {
        const std::string binding = *variable;
        const std::string name = variableName(binding);
        bool replaced = std::find(unset.begin(), unset.end(), name) != unset.end();
        for (const std::string& added : environment)
            replaced = replaced || variableName(added) == name;
        if (!replaced)
            bindings.push_back(binding);
    }
    bindings.insert(bindings.end(), environment.begin(), environment.end());
    for (const std::string& fallback : defaults) {
        const std::string name = variableName(fallback);
        if (std::none_of(bindings.begin(), bindings.end(),
                         [&name](const std::string& binding) { return variableName(binding) == name; }))
            bindings.push_back(fallback);
    }

    std::vector<char*> envp;
    for (std::string& binding : bindings)
        envp.push_back(binding.data());
    envp.push_back(nullptr);

    std::vector<std::string> args = arguments;
    if (args.empty())
        args.push_back(program);
    std::vector<char*> argv;
    for (std::string& argument : args)
        argv.push_back(argument.data());
    argv.push_back(nullptr);

    // A bare name is looked up on PATH, as QProcess does.
    std::string executable = program;
    if (program.find('/') == std::string::npos) {
        const std::string found = Lingmo::base::paths::findExecutable(program);
        if (!found.empty())
            executable = found;
    }

    // Tells the parent whether exec failed.
    int errorPipe[2];
    if (pipe2(errorPipe, O_CLOEXEC) != 0)
        return false;

    const std::string directory = _workingDirectory;
    const int slave = _slave;

    const pid_t pid = fork();
    if (pid < 0) {
        ::close(errorPipe[0]);
        ::close(errorPipe[1]);
        return false;
    }

    if (pid == 0) {
        // Signals as a new program expects them: default handlers, nothing blocked.
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_handler = SIG_DFL;
        action.sa_flags = 0;
        for (int signal = 1; signal < NSIG; signal++)
            sigaction(signal, &action, nullptr);
        sigset_t mask;
        sigemptyset(&mask);
        sigprocmask(SIG_SETMASK, &mask, nullptr);

        // Session leader with the pty as its controlling terminal, in front.
        setsid();
        ioctl(slave, TIOCSCTTY, 0);
        const pid_t group = getpid();
        tcsetpgrp(slave, group);

        dup2(slave, 0);
        dup2(slave, 1);
        dup2(slave, 2);
        // Nothing else of this process reaches the program.
        syscall(SYS_close_range, 3U, ~0U, CLOSE_RANGE_CLOEXEC);

        if (!directory.empty() && chdir(directory.c_str()) != 0) {
            const int error = errno;
            (void)!write(errorPipe[1], &error, sizeof(error));
            _exit(127);
        }

        execve(executable.c_str(), argv.data(), envp.data());
        const int error = errno;
        (void)!write(errorPipe[1], &error, sizeof(error));
        _exit(127);
    }

    ::close(errorPipe[1]);
    int error = 0;
    ssize_t got;
    do {
        got = read(errorPipe[0], &error, sizeof(error));
    } while (got < 0 && errno == EINTR);
    ::close(errorPipe[0]);

    if (got > 0) {
        waitpid(pid, nullptr, 0);
        LM_WARNING(qtermwidgetLogger) << "Cannot start" << program << ":" << strerror(error);
        return false;
    }

    _pid = pid;
    _pidFd = int(syscall(SYS_pidfd_open, pid, 0));
    if (_pidFd >= 0) {
        fcntl(_pidFd, F_SETFD, FD_CLOEXEC);
        _pidWatch = _loop.watch(_pidFd, EPOLLIN, [this](uint32_t) { reap(false); });
    }

    // Writable by the user only, as Konsole leaves a session it is reachable through.
    struct stat sbuf;
    const char* name = ptsname(_master);
    if (name && stat(name, &sbuf) == 0)
        chmod(name, sbuf.st_mode & ~(S_IWGRP | S_IWOTH));

    return true;
}

void PtyProcess::reap(bool wait)
{
    if (_pid <= 0)
        return;

    int status = 0;
    const pid_t result = waitpid(_pid, &status, wait ? 0 : WNOHANG);
    if (result != _pid)
        return;

    if (_pidWatch)
        _loop.unwatch(_pidWatch);
    _pidWatch = 0;
    if (_pidFd >= 0)
        ::close(_pidFd);
    _pidFd = -1;
    _pid = 0;

    // What the program wrote last comes before its end, as QProcess delivers it.
    readMaster();

    const bool crashed = WIFSIGNALED(status);
    const int code = crashed ? WTERMSIG(status) : WEXITSTATUS(status);
    finished(code, crashed);
}

bool PtyProcess::waitForFinished(int msecs)
{
    if (_pid <= 0)
        return true;

    if (_pidFd >= 0) {
        struct pollfd descriptor = {_pidFd, POLLIN, 0};
        int ready;
        do {
            ready = poll(&descriptor, 1, msecs);
        } while (ready < 0 && errno == EINTR);
        if (ready <= 0)
            return false;
    }
    reap(true);
    return _pid <= 0;
}

int PtyProcess::foregroundProcessGroup() const
{
    if (_master >= 0) {
        const int pid = tcgetpgrp(_master);
        if (pid != -1)
            return pid;
    }
    return 0;
}

void PtyProcess::sendData(const char* data, int length)
{
    if (length <= 0 || _master < 0)
        return;

    _pending.append(data, size_t(length));
    writePending();
}

void PtyProcess::writePending()
{
    while (!_pending.empty() && _master >= 0) {
        const ssize_t written = ::write(_master, _pending.data(), _pending.size());
        if (written > 0) {
            _pending.erase(0, size_t(written));
            continue;
        }
        if (written < 0 && errno == EINTR)
            continue;
        if (written < 0 && errno != EAGAIN) {
            LM_WARNING(qtermwidgetLogger) << "Could not send input data to terminal process.";
            _pending.clear();
        }
        break;
    }
    if (_masterWatch)
        _loop.setWatchEvents(_masterWatch, _pending.empty() ? EPOLLIN : EPOLLIN | EPOLLOUT);
}

void PtyProcess::readMaster()
{
    if (_master < 0)
        return;

    for (;;) {
        int available = 0;
        if (ioctl(_master, FIONREAD, &available) != 0 || available <= 0)
            available = 4096;
        _readBuffer.resize(size_t(available));
        const ssize_t got = ::read(_master, _readBuffer.data(), _readBuffer.size());
        if (got < 0 && errno == EINTR)
            continue;
        if (got <= 0)
            return;
        receivedData(_readBuffer.data(), int(got));
        if (_master < 0 || size_t(got) < _readBuffer.size())
            return;
    }
}

}

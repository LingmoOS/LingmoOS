// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/process.h"

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <memory>
#include <linux/close_range.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/file.h"

namespace Lingmo::base {

namespace {

// What a new program starts with. The pipes' ends become its standard input,
// output and error; -1 keeps this process's.
struct Spawn
{
    char* const* argv = nullptr;
    // Null keeps this process's environment.
    char* const* envp = nullptr;
    const char* workingDirectory = nullptr;
    int input = -1;
    int output = -1;
    int error = -1;
    int inherited = -1;
    bool newSession = false;
    bool dropsOomProtection = false;
    // Set by the child when it cannot start the program.
    int failure = 0;
};

} // namespace

// The child shares this process's memory until exec and may not take a lock
// another thread holds: system calls only.
static bool placeFd(int from, int to)
{
    if (from == to)
        return fcntl(to, F_SETFD, 0) == 0;
    return dup2(from, to) == to;
}

static int runSpawnedChild(void* data)
{
    Spawn& spawn = *static_cast<Spawn*>(data);

    // The program starts with default handlers and no signal blocked, whatever
    // this process changed; the parent blocked them all around the clone.
    struct sigaction defaults = {};
    defaults.sa_handler = SIG_DFL;
    for (int signal = 1; signal < NSIG; ++signal)
        sigaction(signal, &defaults, nullptr);
    sigset_t none;
    sigemptyset(&none);
    sigprocmask(SIG_SETMASK, &none, nullptr);

    bool ready = !spawn.newSession || setsid() >= 0;
    ready = ready && (spawn.input < 0 || placeFd(spawn.input, STDIN_FILENO));
    ready = ready && (spawn.output < 0 || placeFd(spawn.output, STDOUT_FILENO));
    ready = ready && (spawn.error < 0 || placeFd(spawn.error, STDERR_FILENO));
    // Close-on-exec rather than closed, so a failed exec can still report.
    ready = ready && (spawn.inherited < 0
                      || (placeFd(spawn.inherited, Process::kInheritedFd)
                          && close_range(Process::kInheritedFd + 1, ~0U, CLOSE_RANGE_CLOEXEC) == 0));
    if (ready && spawn.dropsOomProtection)
        dropOomProtection();
    ready = ready && (!spawn.workingDirectory || chdir(spawn.workingDirectory) == 0);
    if (ready) {
        if (spawn.envp)
            execvpe(spawn.argv[0], spawn.argv, spawn.envp);
        else
            execvp(spawn.argv[0], spawn.argv);
    }
    spawn.failure = errno ? errno : ENOEXEC;
    _exit(127);
}

// As posix_spawn does it: the child runs on a stack of its own in this
// process's memory, and this thread waits until it has called exec, so a large
// process does not copy its page tables for every program it starts.
static pid_t spawnProcess(Spawn& spawn, size_t argumentCount)
{
    const size_t stackSize = 256 * 1024 + argumentCount * sizeof(char*);
    void* stack = mmap(nullptr, stackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        return -1;

    sigset_t all;
    sigset_t previous;
    sigfillset(&all);
    pthread_sigmask(SIG_SETMASK, &all, &previous);
    spawn.failure = 0;
    pid_t pid = clone(runSpawnedChild, static_cast<char*>(stack) + stackSize, CLONE_VM | CLONE_VFORK | SIGCHLD,
                      &spawn);
    const int cloneError = errno;
    pthread_sigmask(SIG_SETMASK, &previous, nullptr);
    munmap(stack, stackSize);

    if (pid < 0) {
        errno = cloneError;
        return -1;
    }
    if (spawn.failure) {
        while (waitpid(pid, nullptr, 0) < 0 && errno == EINTR) {
        }
        errno = spawn.failure;
        return -1;
    }
    return pid;
}

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

static std::vector<char*> argumentVector(const std::string& program, const std::vector<std::string>& arguments)
{
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(program.c_str()));
    for (const std::string& argument : arguments)
        argv.push_back(const_cast<char*>(argument.c_str()));
    argv.push_back(nullptr);
    return argv;
}

// Through syscall(): glibc 2.41's sys/pidfd.h lacks the C linkage C++ needs.
static int openPidfd(pid_t pid)
{
    return int(syscall(SYS_pidfd_open, pid, 0));
}

static int signalPidfd(int pidfd, int signal)
{
    return int(syscall(SYS_pidfd_send_signal, pidfd, signal, nullptr, 0));
}

static void closeFd(int& fd)
{
    if (fd >= 0)
        ::close(fd);
    fd = -1;
}

// Collects a finished program's status from its pidfd; -1 while it runs.
static int reap(int pidfd, bool* crashed)
{
    siginfo_t info{};
    if (waitid(P_PIDFD, id_t(pidfd), &info, WEXITED | WNOHANG) != 0 || info.si_pid == 0)
        return -1;
    *crashed = info.si_code == CLD_KILLED || info.si_code == CLD_DUMPED;
    return info.si_status;
}

Process::Process(EventLoop* loop)
    : m_loop(loop ? *loop : *EventLoop::current())
{
}

Process::~Process()
{
    if (m_pid > 0) {
        ::kill(m_pid, SIGKILL);
        siginfo_t info{};
        waitid(P_PIDFD, id_t(m_pidfd), &info, WEXITED);
    }
    closeWatches();
}

void Process::setWorkingDirectory(std::string directory)
{
    m_workingDirectory = std::move(directory);
}

void Process::setStandardInput(std::string data)
{
    m_input = std::move(data);
}

void Process::setStandardInputOpen(bool open)
{
    m_inputOpen = open;
}

bool Process::writeStandardInput(std::string_view data)
{
    // Closed, though what was queued before may still be on its way.
    if (m_stdin < 0 || m_pid <= 0 || !m_inputOpen)
        return false;
    m_input.erase(0, m_inputWritten);
    m_inputWritten = 0;
    m_input.append(data);
    writeInput();
    return true;
}

void Process::closeStandardInput()
{
    m_inputOpen = false;
    if (m_stdin >= 0 && m_inputWritten >= m_input.size())
        writeInput();
}

void Process::setEnvironment(std::vector<std::string> environment)
{
    m_environment = std::move(environment);
}

void Process::setOutputForwarded(bool forwarded)
{
    m_outputForwarded = forwarded;
}

void Process::setStandardErrorForwarded(bool forwarded)
{
    m_errorForwarded = forwarded;
}

void Process::setInheritedFd(int fd)
{
    m_inheritedFd = fd;
}

void Process::setOomProtectionDropped(bool dropped)
{
    m_dropsOomProtection = dropped;
}

bool Process::start(const std::string& program, const std::vector<std::string>& arguments)
{
    if (m_pid > 0)
        return false;
    closeWatches();
    m_output.clear();
    m_error.clear();
    m_inputWritten = 0;

    int input[2] = {-1, -1};
    int output[2] = {-1, -1};
    int error[2] = {-1, -1};
    const auto closePipes = [&] {
        for (int* pipe : {input, output, error}) {
            closeFd(pipe[0]);
            closeFd(pipe[1]);
        }
    };
    const bool collectsErrors = !m_outputForwarded && !m_errorForwarded;
    if (pipe2(input, O_CLOEXEC) != 0 || (!m_outputForwarded && pipe2(output, O_CLOEXEC) != 0)
        || (collectsErrors && pipe2(error, O_CLOEXEC) != 0)) {
        closePipes();
        return false;
    }

    // A copy well above kInheritedFd, so that dup2 always moves it and
    // clears close-on-exec in the child.
    int inherited = -1;
    if (m_inheritedFd >= 0) {
        inherited = fcntl(m_inheritedFd, F_DUPFD_CLOEXEC, 10);
        if (inherited < 0) {
            closePipes();
            return false;
        }
    }

    std::vector<char*> argv = argumentVector(program, arguments);
    std::vector<char*> envp;
    for (const std::string& entry : m_environment)
        envp.push_back(const_cast<char*>(entry.c_str()));
    envp.push_back(nullptr);
    Spawn spawn;
    spawn.argv = argv.data();
    spawn.envp = m_environment.empty() ? nullptr : envp.data();
    spawn.workingDirectory = m_workingDirectory.empty() ? nullptr : m_workingDirectory.c_str();
    spawn.input = input[0];
    spawn.output = m_outputForwarded ? -1 : output[1];
    spawn.error = collectsErrors ? error[1] : -1;
    spawn.inherited = inherited;
    spawn.dropsOomProtection = m_dropsOomProtection;
    const pid_t pid = spawnProcess(spawn, argv.size());
    closeFd(inherited);
    closeFd(input[0]);
    closeFd(output[1]);
    closeFd(error[1]);
    if (pid < 0) {
        closePipes();
        return false;
    }

    m_pid = pid;
    m_pidfd = openPidfd(pid);
    m_stdin = input[1];
    m_stdout = output[0];
    m_stderr = error[0];
    for (int fd : {m_stdin, m_stdout, m_stderr}) {
        if (fd >= 0)
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    }

    if (m_stdout >= 0) {
        m_stdoutWatch = m_loop.watch(m_stdout, EPOLLIN, [this](uint32_t) {
            readAvailable(m_stdout, m_output, onStandardOutput);
        });
    }
    if (m_stderr >= 0) {
        m_stderrWatch = m_loop.watch(m_stderr, EPOLLIN, [this](uint32_t) {
            readAvailable(m_stderr, m_error, onStandardError);
        });
    }
    if (m_input.empty() && !m_inputOpen)
        closeFd(m_stdin);
    else if (!m_input.empty())
        watchInput();
    m_pidWatch = m_loop.watch(m_pidfd, EPOLLIN, [this](uint32_t) { finish(); });
    return true;
}

void Process::readAvailable(int fd, std::string& into, const std::function<void(std::string_view)>& callback)
{
    char buffer[65536];
    while (true) {
        const ssize_t n = ::read(fd, buffer, sizeof(buffer));
        if (n > 0) {
            into.append(buffer, size_t(n));
            if (callback)
                callback(std::string_view(buffer, size_t(n)));
            continue;
        }
        if (n < 0 && errno == EINTR)
            continue;
        // At the end, the pipe stays readable; stop watching it.
        if (n == 0) {
            if (fd == m_stdout && m_stdoutWatch) {
                m_loop.unwatch(m_stdoutWatch);
                m_stdoutWatch = 0;
            } else if (fd == m_stderr && m_stderrWatch) {
                m_loop.unwatch(m_stderrWatch);
                m_stderrWatch = 0;
            }
        }
        return;
    }
}

void Process::watchInput()
{
    if (!m_stdinWatch)
        m_stdinWatch = m_loop.watch(m_stdin, EPOLLOUT, [this](uint32_t) { writeInput(); });
}

// A write to a pipe nobody reads raises SIGPIPE; it is blocked around the
// write and a pending one taken back, so the program only sees EPIPE.
void Process::writeInput()
{
    bool failed = false;
    while (m_inputWritten < m_input.size()) {
        const ssize_t n = file::writeWithoutSigpipe(m_stdin, m_input.data() + m_inputWritten,
                                              m_input.size() - m_inputWritten);
        if (n > 0) {
            m_inputWritten += size_t(n);
            continue;
        }
        if (n < 0 && errno == EINTR)
            continue;
        if (n < 0 && errno == EAGAIN) {
            watchInput();
            return;
        }
        failed = true;
        break;
    }
    if (m_stdinWatch) {
        m_loop.unwatch(m_stdinWatch);
        m_stdinWatch = 0;
    }
    m_input.clear();
    m_inputWritten = 0;
    if (m_inputOpen && !failed)
        return;
    closeFd(m_stdin);
}

void Process::finish()
{
    bool crashed = false;
    const int status = reap(m_pidfd, &crashed);
    if (status < 0)
        return;
    // Output still in the pipes was written before the end.
    if (m_stdout >= 0)
        readAvailable(m_stdout, m_output, onStandardOutput);
    if (m_stderr >= 0)
        readAvailable(m_stderr, m_error, onStandardError);
    m_pid = -1;
    closeWatches();
    if (auto finished = onFinished)
        finished(status, crashed);
}

void Process::closeWatches()
{
    for (int* watch : {&m_pidWatch, &m_stdinWatch, &m_stdoutWatch, &m_stderrWatch}) {
        if (*watch)
            m_loop.unwatch(*watch);
        *watch = 0;
    }
    closeFd(m_pidfd);
    closeFd(m_stdin);
    closeFd(m_stdout);
    closeFd(m_stderr);
}

bool Process::sendSignal(int signal)
{
    return m_pid > 0 && signalPidfd(m_pidfd, signal) == 0;
}

bool Process::waitForFinished(std::chrono::milliseconds timeout)
{
    if (m_pid <= 0)
        return true;
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (m_pid > 0) {
        const auto now = std::chrono::steady_clock::now();
        const auto left = std::chrono::ceil<std::chrono::milliseconds>(deadline - now);
        // A program blocked on a full output pipe, or waiting for the rest of
        // its input, ends only when the pipes keep moving.
        pollfd descriptors[4] = {{m_pidfd, POLLIN, 0}};
        nfds_t count = 1;
        if (m_stdoutWatch)
            descriptors[count++] = {m_stdout, POLLIN, 0};
        if (m_stderrWatch)
            descriptors[count++] = {m_stderr, POLLIN, 0};
        if (m_stdinWatch)
            descriptors[count++] = {m_stdin, POLLOUT, 0};
        const int ready = ::poll(descriptors, count, int(std::max<int64_t>(left.count(), 0)));
        if (ready < 0 && errno == EINTR)
            continue;
        if (ready < 0 || (ready == 0 && std::chrono::steady_clock::now() >= deadline))
            return false;
        if (ready == 0)
            continue;
        for (nfds_t i = 1; i < count; ++i) {
            if (!descriptors[i].revents)
                continue;
            if (descriptors[i].fd == m_stdout)
                readAvailable(m_stdout, m_output, onStandardOutput);
            else if (descriptors[i].fd == m_stderr)
                readAvailable(m_stderr, m_error, onStandardError);
            else if (descriptors[i].fd == m_stdin)
                writeInput();
        }
        if (descriptors[0].revents)
            finish();
    }
    return true;
}

bool Process::terminate()
{
    return sendSignal(SIGTERM);
}

bool Process::kill()
{
    return sendSignal(SIGKILL);
}

pid_t Process::startDetached(const std::string& program, const std::vector<std::string>& arguments,
                             const std::string& workingDirectory, const std::vector<std::string>& environment)
{
    std::vector<char*> argv = argumentVector(program, arguments);
    std::vector<char*> envp;
    if (!environment.empty()) {
        const auto overridden = [&environment](std::string_view entry) {
            const std::string_view name = entry.substr(0, entry.find('=') + 1);
            return std::any_of(environment.begin(), environment.end(),
                               [name](const std::string& set) { return set.starts_with(name); });
        };
        for (char** entry = environ; *entry; ++entry) {
            if (!overridden(*entry))
                envp.push_back(*entry);
        }
        for (const std::string& entry : environment)
            envp.push_back(const_cast<char*>(entry.c_str()));
        envp.push_back(nullptr);
    }
    Spawn spawn;
    spawn.argv = argv.data();
    spawn.envp = envp.empty() ? nullptr : envp.data();
    spawn.workingDirectory = workingDirectory.empty() ? nullptr : workingDirectory.c_str();
    spawn.newSession = true;
    spawn.dropsOomProtection = true;
    const pid_t pid = spawnProcess(spawn, argv.size());
    if (pid < 0)
        return -1;
    // Reaped when it ends, so it never lingers as a zombie; after this
    // process, init does it.
    const int pidfd = openPidfd(pid);
    EventLoop* loop = EventLoop::current();
    if (pidfd >= 0 && loop) {
        auto watch = std::make_shared<int>(0);
        *watch = loop->watch(pidfd, EPOLLIN, [loop, pidfd, watch](uint32_t) {
            bool crashed = false;
            if (reap(pidfd, &crashed) < 0)
                return;
            loop->unwatch(*watch);
            ::close(pidfd);
        });
    } else if (pidfd >= 0) {
        ::close(pidfd);
    }
    return pid;
}

} // namespace Lingmo::base

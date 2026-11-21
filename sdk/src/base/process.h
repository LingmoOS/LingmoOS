// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <vector>

namespace Lingmo::base {

class EventLoop;

// Takes the oom_score_adj value in the file at `path` back to 0 when it is
// below, and never lowers it: what the session protects from the
// out-of-memory killer, the programs it starts need not share. System calls
// only, so safe between fork and exec.
void dropOomProtection(const char* path = "/proc/self/oom_score_adj");

// A program run in the background: its output is collected and its end reported
// on the loop. The program is looked up on PATH; its environment is this
// process's unless setEnvironment() says otherwise.
class Process
{
public:
    explicit Process(EventLoop* loop = nullptr);
    // Kills a program still running and waits for it.
    ~Process();

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    void setWorkingDirectory(std::string directory);
    // Written to the program's input, which is closed after it unless
    // setStandardInputOpen(true).
    void setStandardInput(std::string data);
    // Before start(): the input stays open after the initial data, for
    // writeStandardInput() and closeStandardInput().
    void setStandardInputOpen(bool open);
    // Queued and written on the loop without blocking; false when the input
    // is closed or the program is not running. A program that stopped
    // reading loses what is left, and raises no SIGPIPE here.
    bool writeStandardInput(std::string_view data);
    // Closes the input once what is queued has been written.
    void closeStandardInput();
    // `NAME=value` entries for the program; empty keeps this process's.
    void setEnvironment(std::vector<std::string> environment);
    // The program's output and errors go to this process's own instead of
    // being collected; onStandardOutput and onStandardError stay silent.
    void setOutputForwarded(bool forwarded);
    // Only the program's errors go to this process's own, as
    // QProcess::ForwardedErrorChannel; its output is still collected.
    void setStandardErrorForwarded(bool forwarded);
    // The program gets `fd` as descriptor kInheritedFd, and every other
    // descriptor above standard error closed. -1 passes none. `fd` must stay
    // open until start() returns.
    static constexpr int kInheritedFd = 3;
    void setInheritedFd(int fd);
    // Before start(): the program does not keep a protection from the
    // out-of-memory killer this process has; see dropOomProtection().
    void setOomProtectionDropped(bool dropped);

    // False when the program cannot be started; onFinished follows otherwise.
    bool start(const std::string& program, const std::vector<std::string>& arguments = {});
    bool isRunning() const { return m_pid > 0; }
    pid_t pid() const { return m_pid; }

    // SIGTERM, SIGKILL, or any signal; false when the program is not running.
    bool terminate();
    bool kill();
    bool sendSignal(int signal);

    // Blocks until the program ends, at most `timeout`; onFinished runs
    // before it returns true. True at once when nothing runs.
    bool waitForFinished(std::chrono::milliseconds timeout);

    // Everything the program wrote so far.
    const std::string& standardOutput() const { return m_output; }
    const std::string& standardError() const { return m_error; }

    // As output arrives.
    std::function<void(std::string_view)> onStandardOutput;
    std::function<void(std::string_view)> onStandardError;
    // The exit code, or the signal that ended the program with crashed set.
    std::function<void(int exitCode, bool crashed)> onFinished;

    // Runs a program on its own, in a session of its own and without this
    // process's protection from the out-of-memory killer; its end is collected
    // in the background. `environment` holds `NAME=value` entries set over
    // this process's. The program's id, or -1.
    static pid_t startDetached(const std::string& program, const std::vector<std::string>& arguments = {},
                               const std::string& workingDirectory = {},
                               const std::vector<std::string>& environment = {});

private:
    void readAvailable(int fd, std::string& into, const std::function<void(std::string_view)>& callback);
    void writeInput();
    void watchInput();
    void finish();
    void closeWatches();

    EventLoop& m_loop;
    std::string m_workingDirectory;
    std::string m_input;
    std::vector<std::string> m_environment;
    bool m_outputForwarded = false;
    bool m_errorForwarded = false;
    bool m_inputOpen = false;
    bool m_dropsOomProtection = false;
    int m_inheritedFd = -1;
    size_t m_inputWritten = 0;
    pid_t m_pid = -1;
    int m_pidfd = -1;
    int m_stdin = -1;
    int m_stdout = -1;
    int m_stderr = -1;
    int m_pidWatch = 0;
    int m_stdinWatch = 0;
    int m_stdoutWatch = 0;
    int m_stderrWatch = 0;
    std::string m_output;
    std::string m_error;
};

} // namespace Lingmo::base

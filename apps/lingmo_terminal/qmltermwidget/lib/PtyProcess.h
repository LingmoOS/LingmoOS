// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PTYPROCESS_H
#define PTYPROCESS_H

#include <string>
#include <sys/types.h>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"

namespace Lingmo::base {
class EventLoop;
}

namespace Konsole
{

// A program on a pseudo terminal, as Konsole's Pty (KPtyProcess) ran it: the
// pty is opened up front, the program becomes the session leader with the
// pty as its controlling terminal and standard input, output and error, and
// output arrives on the event loop.
class PtyProcess
{
public:
    PtyProcess();
    ~PtyProcess();

    PtyProcess(const PtyProcess&) = delete;
    PtyProcess& operator=(const PtyProcess&) = delete;

    // Applied at start, and at once when running.
    void setWindowSize(int lines, int columns);
    Lingmo::base::Size windowSize() const;
    void setFlowControlEnabled(bool enable);
    bool flowControlEnabled() const;
    void setUtf8Mode(bool enable);
    void setErase(char erase);
    char erase() const;

    void setWorkingDirectory(const std::string& directory);

    // Starts `program` (a path, or a name looked up on PATH) with `arguments`,
    // the first of which is the program's argv[0], in this process's environment
    // with `environment` (NAME=value) on top, the names in `unset` removed and
    // `defaults` added where the environment has no such name. False when it
    // cannot start.
    bool start(const std::string& program, const std::vector<std::string>& arguments,
               const std::vector<std::string>& environment, const std::vector<std::string>& unset = {},
               const std::vector<std::string>& defaults = {});

    bool isRunning() const { return _pid > 0; }
    pid_t processId() const { return _pid; }
    // The process group in front on the terminal, or 0.
    int foregroundProcessGroup() const;

    // Queued and written as the pty takes it.
    void sendData(const char* data, int length);

    // As QProcess::waitForFinished(): false when the process still runs after `msecs`.
    bool waitForFinished(int msecs);
    void closePty();

    int masterFd() const { return _master; }
    int slaveFd() const { return _slave; }

    Lingmo::base::Signal<const char*, int> receivedData;
    // The exit code, and whether the program crashed (killed by a signal).
    Lingmo::base::Signal<int, bool> finished;

private:
    void applyAttributes();
    void readMaster();
    void writePending();
    void reap(bool wait);

    Lingmo::base::EventLoop& _loop;
    int _master = -1;
    int _slave = -1;
    int _masterWatch = 0;
    pid_t _pid = 0;
    int _pidFd = -1;
    int _pidWatch = 0;
    std::string _workingDirectory;
    std::string _pending;
    std::vector<char> _readBuffer;

    int _windowColumns = 0;
    int _windowLines = 0;
    char _eraseChar = 0;
    bool _xonXoff = true;
    bool _utf8 = true;
};

}

#endif // PTYPROCESS_H

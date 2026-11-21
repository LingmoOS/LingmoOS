// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shellprocess.h"

#include <unistd.h>

#include "common/environment.h"
#include "journalstream.h"
#include "logs.h"
#include "shellconnection.h"

using namespace std::chrono_literals;

namespace Lingmo {

ShellProcess::ShellProcess()
{
    m_process.setOutputForwarded(true);
    m_process.onFinished = [this](int code, bool crashed) {
        LM_INFO(sessionLog()) << m_program << "exited:" << code
                             << (crashed ? "(crashed)" : "(normally)");
        if (onFinished)
            onFinished(code, crashed);
    };
}

ShellProcess::~ShellProcess()
{
    stop();
}

bool ShellProcess::startForSession(const QString &program, const QString &socket,
                                   ShellConnection *connection)
{
    if (m_process.isRunning())
        return false;

    Lingmo::Core::Environment environment = Lingmo::Core::processEnvironment();
    environment["WAYLAND_DISPLAY"] = socket.toStdString();
    environment.erase("WAYLAND_SOCKET");

    QStringList arguments{QStringLiteral("--daemon")};
    int shellFd = -1;
    if (connection)
        shellFd = connection->createShellConnection();

    if (shellFd >= 0) {
        // The session's copy is close-on-exec, so it cannot leak into anything
        // else the session starts; the daemon inherits it as its only extra
        // descriptor.
        arguments.append(QStringLiteral("--desktop-socket"));
        arguments.append(QString::number(Lingmo::Base::Process::kInheritedFd));
    } else {
        LM_WARNING(sessionLog()) << "Starting" << program
                                 << "without a privileged connection; there will be no desktop";
    }

    m_program = program;
    m_process.setInheritedFd(shellFd);
    m_process.setEnvironment(Lingmo::Core::environmentEntries(environment));
    const bool started = startWithOwnJournalStream(m_process, program, arguments);
    if (started)
        LM_INFO(sessionLog()) << "Session started" << program << "PID" << m_process.pid();
    else
        LM_WARNING(sessionLog()) << program << "failed to start";

    // The session's own copy is no longer needed once the child has it.
    if (shellFd >= 0)
        ::close(shellFd);
    return started;
}

void ShellProcess::stop()
{
    if (!m_process.isRunning())
        return;

    m_process.terminate();
    if (!m_process.waitForFinished(3000ms)) {
        m_process.kill();
        m_process.waitForFinished(1000ms);
    }
}

} // namespace Lingmo

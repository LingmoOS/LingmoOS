// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compositorprocess.h"

#include "base/event_loop.h"
#include "journalstream.h"

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace std::chrono_literals;

namespace Lingmo {

CompositorProcess::CompositorProcess()
{
    m_process.setOutputForwarded(true);
    m_timeout.setSingleShot(true);
    m_timeout.onTimeout = [this] {
        fail(QStringLiteral("Compositor startup timed out"));
    };
    m_process.onFinished = [this](int code, bool crashed) {
        closeReadyPipe();
        if (m_stopping || m_failed)
            return;
        if (!m_ready)
            fail(QStringLiteral("Compositor exited before becoming ready"));
        else if (onExited)
            onExited(crashed ? 1 : code);
    };
}

CompositorProcess::~CompositorProcess()
{
    stop();
}

void CompositorProcess::closeReadyPipe()
{
    m_timeout.stop();
    if (m_readWatch) {
        Lingmo::Base::EventLoop::current()->unwatch(m_readWatch);
        m_readWatch = 0;
    }
    if (m_readFd >= 0) {
        close(m_readFd);
        m_readFd = -1;
    }
}

void CompositorProcess::fail(const QString &reason)
{
    if (m_failed || m_stopping)
        return;
    m_failed = true;
    stop();
    if (onFailed)
        onFailed(reason);
}

void CompositorProcess::readReady()
{
    char marker = 0;
    const auto count = read(m_readFd, &marker, 1);
    if (count < 0)
        return;
    if (count != 1 || marker != 'R') {
        fail(QStringLiteral("Compositor did not confirm startup"));
        return;
    }
    closeReadyPipe();
    m_ready = true;
    if (onReady)
        onReady();
}

void CompositorProcess::start(const QString &program, const QString &socket,
                              const QString &controlSocket, int timeoutMs,
                              const QString &xwaylandReadyFile)
{
    if (m_process.isRunning() || m_readFd >= 0)
        return;
    m_ready = false;
    m_stopping = false;
    m_failed = false;

    int fds[2];
    if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0) {
        fail(QStringLiteral("Cannot create compositor startup pipe"));
        return;
    }
    m_readFd = fds[0];
    const int writeFd = fds[1];

    // The write end is the compositor's only inherited descriptor.
    QStringList arguments{
        QStringLiteral("--socket"), socket,
        QStringLiteral("--ready-fd"),
        QString::number(Lingmo::Base::Process::kInheritedFd),
    };
    if (!controlSocket.isEmpty()) {
        arguments.append(QStringLiteral("--shell-control-socket"));
        arguments.append(controlSocket);
    }
    if (!xwaylandReadyFile.isEmpty()) {
        arguments.append(QStringLiteral("--xwayland-ready-file"));
        arguments.append(xwaylandReadyFile);
    }
    m_readWatch = Lingmo::Base::EventLoop::current()->watch(m_readFd, EPOLLIN, [this](uint32_t) {
        readReady();
    });
    m_process.setInheritedFd(writeFd);
    const bool started = startWithOwnJournalStream(m_process, program, arguments);
    close(writeFd);
    if (!started) {
        fail(QStringLiteral("Cannot start ") + program);
        return;
    }
    if (!m_ready && !m_failed && !m_stopping)
        m_timeout.start(std::chrono::milliseconds(timeoutMs));
}

void CompositorProcess::stop()
{
    m_stopping = true;
    closeReadyPipe();
    if (m_process.isRunning()) {
        m_process.terminate();
        if (!m_process.waitForFinished(3000ms)) {
            m_process.kill();
            m_process.waitForFinished(1000ms);
        }
    }
    m_ready = false;
}

} // namespace Lingmo

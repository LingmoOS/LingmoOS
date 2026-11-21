// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QString>

#include "base/process.h"
#include "base/timer.h"

namespace Lingmo {

// Session lifecycle only. No window-management or shell UI traffic crosses here.
class CompositorProcess final
{
public:
    CompositorProcess();
    ~CompositorProcess();

    void start(const QString &program, const QString &socket,
               const QString &controlSocket,
               int timeoutMs = 15000,
               const QString &xwaylandReadyFile = {});
    void stop();

    bool isReady() const { return m_ready; }

    std::function<void()> onReady;
    std::function<void(const QString &reason)> onFailed;
    std::function<void(int code)> onExited;

private:
    void closeReadyPipe();
    void fail(const QString &reason);
    void readReady();

    Lingmo::Base::Process m_process;
    Lingmo::Base::Timer m_timeout;
    int m_readFd = -1;
    int m_readWatch = 0;
    bool m_ready = false;
    bool m_stopping = false;
    bool m_failed = false;
};

} // namespace Lingmo

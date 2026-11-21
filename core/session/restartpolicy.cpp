// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "restartpolicy.h"
#include "logs.h"

namespace Lingmo {

RestartPolicy::RestartPolicy(const QString &name, std::function<void()> restart)
    : RestartPolicy(name, std::move(restart), Limits())
{
}

RestartPolicy::RestartPolicy(const QString &name, std::function<void()> restart,
                             const Limits &limits)
    : m_name(name)
    , m_restart(std::move(restart))
    , m_limits(limits)
{
    m_timer.setSingleShot(true);
    m_timer.onTimeout = [this] {
        if (!m_stopped)
            m_restart();
    };
}

void RestartPolicy::started()
{
    m_startedAt = std::chrono::steady_clock::now();
}

void RestartPolicy::exited(int code, bool crashed)
{
    if (m_stopped || m_timer.isActive())
        return;

    if (m_startedAt
        && std::chrono::steady_clock::now() - *m_startedAt
               >= std::chrono::milliseconds(m_limits.stableUptimeMs))
        m_restarts = 0;

    if (m_restarts >= m_limits.maxRestarts) {
        LM_CRITICAL(sessionLog()) << m_name << "failed" << m_restarts
                                  << "times; not restarting it again";
        m_gaveUp = true;
        return;
    }

    const int delay = m_limits.delayMs + m_restarts * m_limits.backoffMs;
    ++m_restarts;
    LM_WARNING(sessionLog()) << m_name << "exited" << code
                             << (crashed ? "(crashed)" : "(normally)")
                             << "- restarting in" << delay << "ms";
    m_timer.start(std::chrono::milliseconds(delay));
}

void RestartPolicy::stop()
{
    m_stopped = true;
    m_timer.stop();
}

bool RestartPolicy::isPending() const
{
    return m_timer.isActive();
}

bool RestartPolicy::hasGivenUp() const
{
    return m_gaveUp;
}

} // namespace Lingmo

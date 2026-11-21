// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>

#include <QString>

#include "base/timer.h"

namespace Lingmo {

// Bounded restarts for a session process.
// The compositor keeps running either way: a session missing one of its
// processes is recoverable, a restart loop is not.
class RestartPolicy
{
public:
    struct Limits {
        int maxRestarts = 5;
        // A process that stayed up this long was not crash-looping; reset the count.
        int64_t stableUptimeMs = 30000;
        int delayMs = 1000;
        int backoffMs = 2000;
    };

    RestartPolicy(const QString &name, std::function<void()> restart);
    RestartPolicy(const QString &name, std::function<void()> restart, const Limits &limits);

    // Call on every start, the first included.
    void started();
    void exited(int code, bool crashed);

    // Cancels a pending restart; later exits are ignored.
    void stop();

    bool isPending() const;
    bool hasGivenUp() const;

private:
    QString m_name;
    std::function<void()> m_restart;
    Limits m_limits;
    Lingmo::Base::Timer m_timer;
    std::optional<std::chrono::steady_clock::time_point> m_startedAt;
    int m_restarts = 0;
    bool m_stopped = false;
    bool m_gaveUp = false;
};

} // namespace Lingmo

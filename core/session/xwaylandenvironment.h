// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QString>

#include "base/file.h"
#include "base/file_watcher.h"
#include "base/timer.h"

namespace Lingmo {

class XwaylandEnvironment final
{
public:
    XwaylandEnvironment();

    QString readinessFile() const;
    void waitForReady(int timeoutMs = 12000);

    std::function<void(bool available)> onResolved;
    std::function<void()> onLost;

private:
    void check();

    Lingmo::Base::file::TemporaryDirectory m_directory;
    Lingmo::Base::FileWatch m_watch;
    Lingmo::Base::Timer m_timeout;
    bool m_waiting = false;
    bool m_available = false;
};

} // namespace Lingmo

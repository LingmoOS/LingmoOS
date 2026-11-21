// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/dbus.h"
#include <functional>
#include <memory>
#include <vector>

namespace Lingmo::WM {
class QuitAll;
}

namespace Lingmo::Session {

// org.lingmo.Applications1, which the session asks to quit every application
// before it logs out, restarts or shuts down.
class ApplicationsService
{
public:
    // `setEnding` puts the docks and panels away while applications quit, and
    // brings them back if one keeps the session from ending. The session bus
    // when no connection is given.
    ApplicationsService(wm::QuitAll *quitAll, std::function<void(bool)> setEnding,
                        base::dbus::Connection *bus = nullptr);
    ApplicationsService(const ApplicationsService &) = delete;
    ApplicationsService &operator=(const ApplicationsService &) = delete;
    ~ApplicationsService();

    bool registerService();

private:
    // QuitAll replies once every application has quit, "quit", or once one
    // has kept the session from ending, "cancelled" or "unresponsive" with
    // its id. Concurrent callers share the answer.
    void quitAll(base::dbus::MethodCall &call);

    wm::QuitAll *m_quitAll;
    std::function<void(bool)> m_setEnding;
    base::dbus::Connection *m_bus;
    std::unique_ptr<base::dbus::Object> m_object;
    std::vector<base::dbus::MethodCall> m_pending;
    bool m_registered = false;
};

} // namespace Lingmo::Session

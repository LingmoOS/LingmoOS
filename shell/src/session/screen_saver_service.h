// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/dbus.h"
#include "base/signal.h"
#include "session/session_state.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::Session {
class SessionController;

// The one org.freedesktop.ScreenSaver object, at /ScreenSaver and
// /org/freedesktop/ScreenSaver: a second object at either path fails to register.
class ScreenSaverService : public SessionObserver
{
public:
    // The session bus when no connection is given.
    explicit ScreenSaverService(SessionController *session,
                                base::dbus::Connection *bus = nullptr);
    ~ScreenSaverService() override;
    bool registerService();
    void onSessionStateChanged(SessionState state) override;

    // Inhibit counts against the compositor's idle inhibition through these.
    // Without them, as in the greeter, Inhibit is refused.
    void setInhibition(std::function<void()> acquire, std::function<void()> release);

    // Locks as the D-Bus Lock does, for callers inside the compositor.
    void Lock();
    bool GetActive() const;

    // Also sent on the bus.
    base::Signal<bool> activeChanged;

private:
    void lock(base::dbus::MethodCall *call);
    void inhibit(base::dbus::MethodCall &call);
    void uninhibit(base::dbus::MethodCall &call);
    void releaseOwner(const std::string &owner);

    SessionController *m_session;
    base::dbus::Connection *m_bus;
    std::vector<std::unique_ptr<base::dbus::Object>> m_objects;
    // A caller that leaves the bus takes all of its cookies with it.
    std::map<std::string, base::dbus::NameWatch> m_owners;
    std::vector<base::dbus::MethodCall> m_pending;
    std::function<void()> m_acquire;
    std::function<void()> m_release;
    std::map<uint32_t, std::string> m_cookies;
    uint32_t m_nextCookie = 0;
    bool m_registered = false;
    bool m_active;
};
} // namespace Lingmo::Session

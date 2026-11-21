// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/dbus.h"
#include "base/timer.h"
#include "session/session_state.h"

namespace Lingmo::Session {
class SessionController;

// Locks the session before the system sleeps, however the sleep was asked
// for: a menu, the lid, idle or systemctl. A logind delay inhibitor holds the
// sleep back until the lock is on every output, so waking never shows the
// desktop. Upstream: ash/system/power/power_event_observer.cc, which delays
// suspend the same way until the lock screen has been composited.
//
// It also keeps logind's LockedHint on the session in step with the lock, as
// Ash tells session_manager that the lock screen was shown or dismissed.
class SleepLock : public SessionObserver
{
public:
    // The system bus when no connection is given.
    explicit SleepLock(SessionController *session, base::dbus::Connection *bus = nullptr);
    SleepLock(const SleepLock &) = delete;
    SleepLock &operator=(const SleepLock &) = delete;
    ~SleepLock() override;

    // Follows logind's PrepareForSleep, takes the inhibitor and from then on
    // sets the session's LockedHint.
    bool start();

    bool holdsInhibitor() const { return m_inhibitor.isValid(); }

    void onSessionStateChanged(SessionState state) override;

private:
    void prepareForSleep(bool sleeping);
    void inhibit();
    void releaseWhenLocked();
    void updateLockedHint(SessionState state);

    SessionController *m_session;
    base::dbus::Connection *m_bus;
    base::dbus::Proxy m_manager;
    base::dbus::Proxy m_logindSession;
    base::dbus::Match m_prepareForSleep;
    base::dbus::PendingCall m_inhibiting;
    base::dbus::UnixFd m_inhibitor;
    // Not from inside the controller's own state change.
    base::Timer m_relock;
    bool m_sleeping = false;
    bool m_lockedHint = false;
    bool m_started = false;
};
} // namespace Lingmo::Session

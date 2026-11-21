// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"
#include "shell/common/pending_calls.h"

namespace Lingmo::Shell {

// Log out, restart and shut down, which the user is asked to confirm first,
// and suspend and lock, which act at once: the user menu's and the power
// dialog's actions, through the session (org.lingmo.Session) and the
// screen saver.
class PowerActions
{
public:
    enum Action {
        None,
        Logout,
        Reboot,
        Shutdown,
    };

    explicit PowerActions(base::dbus::Connection *bus = base::dbus::Connection::session());

    // What the confirmation dialog asks about; None while it is not up.
    Action pending() const { return m_pending; }

    // Shows the confirmation for the action.
    void request(Action action);
    // The dialog's answers. The pending action clears on the next turn of the
    // loop, as the dialog answering is still on the stack.
    void confirm();
    void cancel();

    void logout();
    void reboot();
    void shutdown();
    void suspend();
    void lockScreen();

    base::Signal<> pendingChanged;
    base::Signal<> requested;

private:
    void setPending(Action action);
    void callSession(const char *method);

    base::dbus::Proxy m_session;
    base::dbus::Proxy m_screenSaver;
    PendingCalls m_calls;
    Action m_pending = None;
    base::Timer m_clear;
};

} // namespace Lingmo::Shell


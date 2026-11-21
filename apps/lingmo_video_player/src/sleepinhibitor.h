// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"
#include "base/signal.h"
#include "base/weak_ptr.h"

#include <cstdint>
#include <map>

// Keeps the screen on while a video plays, and the computer awake while
// audio plays.
class SleepInhibitor
{
public:
    enum class Mode {
        Off,
        // The screen stays on (org.freedesktop.ScreenSaver).
        Screen,
        // Only suspend is held off (logind), the screen may still blank.
        Suspend,
    };

    // The buses default to the loop's shared session and system ones.
    explicit SleepInhibitor(Lingmo::base::dbus::Connection* session = nullptr,
                            Lingmo::base::dbus::Connection* system = nullptr);
    ~SleepInhibitor();

    Mode mode() const { return m_mode; }
    void setMode(Mode mode);
    // Takes the held inhibition again, its reasons in the language of the
    // moment.
    void retranslate();

    // The ScreenSaver cookie or logind lock is held.
    bool isHeld() const { return m_cookie != 0 || m_suspendLock.isValid(); }

    Lingmo::base::Signal<> modeChanged;
    // A held inhibition arrived or went.
    Lingmo::base::Signal<> heldChanged;

private:
    // Asks for mode's inhibition; an answer for an older generation is undone.
    void acquire(Mode mode, uint64_t generation);
    void release();
    void finish(uint64_t call);

    Lingmo::base::dbus::Connection* m_session;
    Lingmo::base::dbus::Connection* m_system;
    Mode m_mode = Mode::Off;
    // Bumped on every change, so a late reply for an older mode is undone.
    uint64_t m_generation = 0;
    uint32_t m_cookie = 0;
    Lingmo::base::dbus::UnixFd m_suspendLock;
    // Kept until answered: a dropped call would leave its inhibition behind.
    std::map<uint64_t, Lingmo::base::dbus::PendingCall> m_calls;
    Lingmo::base::WeakPtrFactory<SleepInhibitor> m_weakFactory{this};
};

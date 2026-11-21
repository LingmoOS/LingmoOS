// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/dbus.h"
#include "input/event_sink.h"
#include <chrono>
#include <optional>

namespace Lingmo::Session {
class SessionController;

// A tap on the power button locks the session; the lock screen then goes dark on
// its own idle timeout. Holding it is left to the firmware's forced power-off.
// logind is asked to leave the key alone, or it would power the machine off.
// Upstream: ash/system/power/power_button_controller.cc, the tablet-mode tap.
class PowerButton : public input::EventSink
{
public:
    // `next` receives every other event. The system bus when no connection
    // is given.
    PowerButton(SessionController *session, input::EventSink *next,
                base::dbus::Connection *bus = nullptr);
    ~PowerButton() override;

    // Takes the key over from logind and follows its PrepareForSleep. Until the
    // inhibitor is held the key passes on untouched.
    bool start();

    bool holdsInhibitor() const { return m_inhibitor.isValid(); }

    // input::EventSink:
    bool onPointerMotion(input::Timestamp timestamp, const base::PointF &location) override;
    bool onPointerButton(input::Timestamp timestamp, const base::PointF &location,
                         uint32_t evdevButton, bool pressed) override;
    bool onPointerAxis(input::Timestamp timestamp, const base::PointF &location,
                       const base::PointF &offset, bool discrete) override;
    bool onPointerAxisStop(input::Timestamp timestamp, const base::PointF &location) override;
    bool onKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed) override;
    bool onSecureKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed) override;
    bool onTouchDown(input::Timestamp timestamp, int32_t id, const base::PointF &location) override;
    bool onTouchMotion(input::Timestamp timestamp, int32_t id, const base::PointF &location) override;
    bool onTouchUp(input::Timestamp timestamp, int32_t id) override;
    void onTouchCancel(input::Timestamp timestamp) override;

private:
    void prepareForSleep(bool sleeping);
    void inhibit();

    SessionController *m_session;
    input::EventSink *m_next;
    base::dbus::Connection *m_bus;
    base::dbus::Proxy m_manager;
    base::dbus::Match m_prepareForSleep;
    base::dbus::PendingCall m_inhibiting;
    base::dbus::UnixFd m_inhibitor;
    std::optional<std::chrono::steady_clock::time_point> m_resumed;
    std::optional<std::chrono::steady_clock::time_point> m_released;
    bool m_ignoreRelease = false;
    bool m_started = false;
};
} // namespace Lingmo::Session

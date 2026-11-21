// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/ash/wm/lock_state_controller.cc, which drops capture,
// gestures and the active window before the lock screen is shown, and on
// chromium/src/ash/login/ui/lock_screen.h, whose saved_clipboard_ gives the lock
// screen a clipboard of its own.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_SESSION_SESSION_TEARDOWN_H_
#define LINGMO_SESSION_SESSION_TEARDOWN_H_

#include <functional>

#include <cstdint>

#include "session/session_controller.h"

namespace Lingmo::Input {
class Seat;
}

namespace Lingmo::WM {
class WindowCycleController;
class WindowManager;
}

namespace Lingmo::Session {

// What has to go the instant the session stops being active. A grab, a popup
// grab, a drag and a cursor surface all outlive the hit test that granted them,
// so none of them may survive into a blocked session; keyboard focus, window
// activation, the clipboard and any capture in flight go with them.
class SessionTeardown : public SessionController::Delegate
{
public:
    SessionTeardown(input::Seat *seat, wm::WindowManager *windowManager);

    // Puts the pointer back on a compositor-owned image, including the KMS
    // cursor plane. Set from main, which owns cursor::Cursor; without it the
    // scene cursor is still cleared, only the hardware plane is not refreshed.
    void setCursorReset(std::function<void()> reset);

    // Fails every capture frame in flight. Set from main, which owns the
    // capture manager.
    void setCaptureAbort(std::function<void()> abort);

    // Cancelled with the block: the modifiers that would complete it are let
    // go of behind the lock screen.
    void setWindowCycle(wm::WindowCycleController *windowCycle);

    void onSessionBlocked() override;
    void onSessionUnblocked() override;

private:
    void clearCursor();

    input::Seat *m_seat = nullptr;
    wm::WindowManager *m_windowManager = nullptr;
    wm::WindowCycleController *m_windowCycle = nullptr;
    // The window active when the session was blocked, by id, which a window
    // closing meanwhile cannot leave dangling.
    uint64_t m_activeBeforeBlock = 0;
    std::function<void()> m_cursorReset;
    std::function<void()> m_captureAbort;
};

} // namespace Lingmo::Session

#endif // LINGMO_SESSION_SESSION_TEARDOWN_H_

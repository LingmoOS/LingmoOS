// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/session_teardown.h"

#include <utility>

#include "clipboard/selection.h"
#include "input/drag.h"
#include "input/seat.h"
#include "wm/activation.h"
#include "wm/window.h"
#include "wm/window_cycle_controller.h"
#include "wm/window_manager.h"

namespace Lingmo::Session {

SessionTeardown::SessionTeardown(input::Seat *seat, wm::WindowManager *windowManager)
    : m_seat(seat)
    , m_windowManager(windowManager)
{
}

void SessionTeardown::setCursorReset(std::function<void()> reset)
{
    m_cursorReset = std::move(reset);
}

void SessionTeardown::setCaptureAbort(std::function<void()> abort)
{
    m_captureAbort = std::move(abort);
}

void SessionTeardown::setWindowCycle(wm::WindowCycleController *windowCycle)
{
    m_windowCycle = windowCycle;
}

void SessionTeardown::clearCursor()
{
    // Dropping pointer focus is what detaches the client's cursor surface from
    // the cursor container; without it a stale wl_surface would keep compositing
    // above the secure scene, which is the one place nothing else covers.
    m_seat->clearPointerFocus();
    m_seat->setCursorOverride(std::nullopt);
    m_seat->setCompositorCursorShape(input::CursorShape::Default);
    m_seat->notifyCursorChanged();
    // The scene cursor is now compositor-owned; the KMS cursor plane still
    // holds the last image it was given and has to be reprogrammed.
    if (m_cursorReset)
        m_cursorReset();
}

void SessionTeardown::onSessionBlocked()
{
    m_windowManager->dismissPopupGrabs();
    m_windowManager->cancelInteractions();
    if (m_windowCycle)
        m_windowCycle->cancelCycling();
    // Cancels the drag and takes its icon out of the scene with it.
    m_seat->drag()->cancel();
    m_seat->setFocusedSurface(nullptr);
    // Keys held now are not suppressed: their releases must still reach the
    // key translator, which holds them in its own codes (see InputManager::onKey).
    const wm::Window *active = m_windowManager->activation()->activeWindow();
    m_activeBeforeBlock = active ? active->id() : 0;
    m_windowManager->activation()->activate(nullptr);
    clearCursor();
    m_seat->selection()->setIsolated(true);
    m_seat->primarySelection()->setIsolated(true);
    if (m_captureAbort)
        m_captureAbort();
}

void SessionTeardown::onSessionUnblocked()
{
    // The window that had the keyboard gets it back, so typing carries on
    // where it stopped; one that closed meanwhile leaves it to the topmost
    // window left. The cursor stays the compositor's until the pointer moves.
    m_seat->selection()->setIsolated(false);
    m_seat->primarySelection()->setIsolated(false);
    clearCursor();
    // A capture left open behind the lock takes the keyboard back first, so
    // the activation below leaves it there.
    if (wm::Window *grab = m_windowManager->keyboardGrab())
        m_windowManager->setKeyboardGrab(grab);
    const uint64_t id = std::exchange(m_activeBeforeBlock, 0);
    if (!id)
        return;
    wm::Activation *activation = m_windowManager->activation();
    wm::Window *window = m_windowManager->windowById(id);
    if (!window || !activation->canActivate(window)) {
        window = nullptr;
        const auto &stacking = m_windowManager->windows();
        for (auto it = stacking.rbegin(); it != stacking.rend(); ++it) {
            // Only the active desk's: another desk's window would take the
            // user there.
            if ((*it)->role() == wm::WindowRole::Normal && (*it)->isVisible()
                && activation->canActivate(*it)) {
                window = *it;
                break;
            }
        }
    }
    if (window)
        activation->activate(window);
}

} // namespace Lingmo::Session

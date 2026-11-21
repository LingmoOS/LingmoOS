// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/accelerator_filter.cc and
//           chromium/src/ash/accelerators/pre_target_accelerator_handler.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_filter.h"

#include <string>

#include <xkbcommon/xkbcommon.h>

#include "accelerators/accelerator_controller.h"
#include "core/logging.h"
#include "input/seat.h"
#include "input/xkb_tracker.h"
#include "wm/activation.h"
#include "wm/window.h"
#include "wm/window_cycle_controller.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::Accelerators {

namespace {

// ash::AcceleratorController::IsSystemKey. The media keys are here too: they
// go to the session's MPRIS player, which is what makes a player in the
// background answer them. An application that wants them for itself binds
// them, and its binding wins over the default table's.
bool isSystemKey(xkb_keysym_t keysym)
{
    switch (keysym) {
    case XKB_KEY_XF86AudioMute:
    case XKB_KEY_XF86AudioLowerVolume:
    case XKB_KEY_XF86AudioRaiseVolume:
    case XKB_KEY_XF86AudioMicMute:
    case XKB_KEY_XF86AudioPlay:
    case XKB_KEY_XF86AudioPause:
    case XKB_KEY_XF86AudioStop:
    case XKB_KEY_XF86AudioPrev:
    case XKB_KEY_XF86AudioNext:
    case XKB_KEY_XF86MonBrightnessDown:
    case XKB_KEY_XF86MonBrightnessUp:
    case XKB_KEY_XF86KbdBrightnessDown:
    case XKB_KEY_XF86KbdBrightnessUp:
        return true;
    default:
        return false;
    }
}

} // namespace

bool usesPositionalShortcuts(const input::XkbTracker &xkb)
{
    // No layout set means xkbcommon's default, which is us.
    const std::string &layout = xkb.layoutName();
    return !layout.empty() && layout.rfind("us", 0) != 0;
}

AcceleratorFilter::AcceleratorFilter(AcceleratorController *controller,
                                     const input::XkbTracker *xkb,
                                     wm::WindowManager *windowManager,
                                     wm::WindowCycleController *windowCycle,
                                     input::EventSink *next)
    : m_controller(controller)
    , m_xkb(xkb)
    , m_windowManager(windowManager)
    , m_windowCycle(windowCycle)
    , m_next(next)
{
    m_repeatTimer.setSingleShot(true);
    m_repeatTimer.onTimeout = [this] { onRepeat(); };
}

AcceleratorFilter::~AcceleratorFilter() = default;

Accelerator AcceleratorFilter::acceleratorForKey(uint32_t evdevCode, bool pressed) const
{
    uint32_t modifiers = NoModifier;
    if (m_xkb->modifierPressed(XKB_MOD_NAME_SHIFT))
        modifiers |= ShiftDown;
    if (m_xkb->modifierPressed(XKB_MOD_NAME_CTRL))
        modifiers |= ControlDown;
    if (m_xkb->modifierPressed(XKB_MOD_NAME_ALT))
        modifiers |= AltDown;
    if (m_xkb->modifierPressed(XKB_MOD_NAME_LOGO))
        modifiers |= MetaDown;
    if (m_xkb->modifierPressed("Mod5"))
        modifiers |= AltGrDown;
    return Accelerator(m_xkb->baseKeysym(evdevCode), evdevCode, modifiers,
                       pressed ? Accelerator::KeyState::Pressed
                               : Accelerator::KeyState::Released);
}

void AcceleratorFilter::setKeyRepeatSettings(const input::KeyRepeatSettings &settings)
{
    m_repeatSettings = settings;
    if (!settings.enabled)
        stopRepeat();
}

void AcceleratorFilter::startRepeat(const Accelerator &accelerator)
{
    if (!m_repeatSettings.enabled || !m_controller->isRepeatable(accelerator))
        return;
    m_repeatAccelerator = accelerator;
    m_repeatTimer.start(m_repeatSettings.delay);
}

void AcceleratorFilter::stopRepeat()
{
    m_repeatTimer.stop();
}

void AcceleratorFilter::onRepeat()
{
    m_repeatTimer.start(m_repeatSettings.interval);
    if (!processAccelerator(m_repeatAccelerator))
        stopRepeat();
}

bool AcceleratorFilter::onPointerMotion(input::Timestamp timestamp, const base::PointF &location)
{
    if (m_windowCycle)
        m_windowCycle->onPointerMotion(location);
    return m_next && m_next->onPointerMotion(timestamp, location);
}

bool AcceleratorFilter::onPointerButton(input::Timestamp timestamp, const base::PointF &location,
                                        uint32_t evdevButton, bool pressed)
{
    m_controller->launcherStateMachine()->onPointerButton(pressed);
    if (pressed && m_windowCycle && m_windowCycle->onPointerButtonPressed(location))
        return true;
    return m_next && m_next->onPointerButton(timestamp, location, evdevButton, pressed);
}

bool AcceleratorFilter::onPointerAxis(input::Timestamp timestamp, const base::PointF &location,
                                      const base::PointF &offset, bool discrete)
{
    return m_next && m_next->onPointerAxis(timestamp, location, offset, discrete);
}

bool AcceleratorFilter::onPointerAxisStop(input::Timestamp timestamp, const base::PointF &location)
{
    return m_next && m_next->onPointerAxisStop(timestamp, location);
}

bool AcceleratorFilter::onTouchDown(input::Timestamp timestamp, int32_t id,
                                    const base::PointF &location)
{
    return m_next && m_next->onTouchDown(timestamp, id, location);
}

bool AcceleratorFilter::onTouchMotion(input::Timestamp timestamp, int32_t id,
                                      const base::PointF &location)
{
    return m_next && m_next->onTouchMotion(timestamp, id, location);
}

bool AcceleratorFilter::onTouchUp(input::Timestamp timestamp, int32_t id)
{
    return m_next && m_next->onTouchUp(timestamp, id);
}

void AcceleratorFilter::onTouchCancel(input::Timestamp timestamp)
{
    if (m_next)
        m_next->onTouchCancel(timestamp);
}

bool AcceleratorFilter::onKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    const Accelerator accelerator = acceleratorForKey(evdevCode, pressed);

    // A key going down ends the repeat, as it does for the seat's.
    if (pressed || evdevCode == m_repeatAccelerator.evdevCode())
        stopRepeat();

    // Ash registers these on aura::Env, ahead of every other pre-target
    // handler, so they see the key whatever happens to it afterwards.
    m_controller->history()->storeCurrentAccelerator(accelerator);
    m_controller->launcherStateMachine()->onKey(accelerator.keysym(), pressed);

    // The way out of a window holding the pointer or the shortcuts; checked
    // before an inhibitor could hand the key to it.
    input::Seat *seat = m_windowManager ? m_windowManager->seat() : nullptr;
    if (pressed && accelerator.isMetaDown() && accelerator.keysym() == XKB_KEY_Escape && seat
        && seat->breakInputCapture()) {
        return true;
    }

    // While cycling, every key that is not the cycle trigger belongs to the
    // cycle: letting go of the modifiers that started it (Alt, or Command)
    // completes it, Escape cancels it. The client saw those modifiers go down,
    // so it gets their release as well.
    if (m_windowCycle && m_windowCycle->isCycling()
        && !m_controller->doesAcceleratorMatchAction(accelerator,
                                                     AcceleratorAction::CycleForwardMru)
        && !m_controller->doesAcceleratorMatchAction(accelerator,
                                                     AcceleratorAction::CycleBackwardMru)) {
        if (!pressed && m_cycleModifiers && !(accelerator.modifiers() & m_cycleModifiers))
            m_windowCycle->completeCycling();
        else if (m_windowCycle->onKey(accelerator.keysym(), pressed))
            return true;
    }

    if (processAccelerator(accelerator)) {
        if (m_windowCycle && m_windowCycle->isCycling())
            m_cycleModifiers = accelerator.modifiers() & ~ShiftDown;
        if (pressed)
            startRepeat(accelerator);
        return true;
    }
    return m_next && m_next->onKey(timestamp, evdevCode, pressed);
}

bool AcceleratorFilter::onSecureKey(input::Timestamp, uint32_t evdevCode, bool pressed)
{
    // Only the system keys: the history, the launcher and the window cycle
    // have nothing to see behind the lock screen.
    const Accelerator accelerator = acceleratorForKey(evdevCode, pressed);
    if (pressed || evdevCode == m_repeatAccelerator.evdevCode())
        stopRepeat();
    if (!isSystemKey(accelerator.keysym()))
        return false;
    m_controller->process(accelerator);
    if (pressed)
        startRepeat(accelerator);
    return true;
}

bool AcceleratorFilter::processAccelerator(const Accelerator &accelerator)
{
    // System keys are always consumed regardless of whether they trigger an
    // accelerator to prevent windows from seeing unexpected key up events.
    if (isSystemKey(accelerator.keysym()) && !canConsumeSystemKeys()) {
        m_controller->process(accelerator);
        return true;
    }
    if (!shouldProcessAcceleratorNow(accelerator))
        return false;
    return m_controller->process(accelerator);
}

bool AcceleratorFilter::canConsumeSystemKeys() const
{
    // Exo's inhibitor is what lets the window have them.
    input::Seat *seat = m_windowManager ? m_windowManager->seat() : nullptr;
    return seat && seat->shortcutsInhibition()->isActive();
}

bool AcceleratorFilter::shouldProcessAcceleratorNow(const Accelerator &accelerator) const
{
    // Reserved accelerators (such as the lock key) always have priority.
    if (m_controller->isReserved(accelerator))
        return true;

    // Exo's inhibitor only lets the window consume system keys. A remote
    // desktop or a virtual machine needs Meta as well, so it gets every key
    // but the reserved ones and Meta+Escape.
    input::Seat *seat = m_windowManager ? m_windowManager->seat() : nullptr;
    if (seat && seat->shortcutsInhibition()->isActive())
        return false;

    // Meta+<key> is never the application's otherwise, fullscreen or not, and
    // neither are the preferred ones.
    if (accelerator.isMetaDown() || accelerator.keysym() == XKB_KEY_Super_L
        || accelerator.keysym() == XKB_KEY_Super_R || m_controller->isPreferred(accelerator)) {
        return true;
    }

    // A full screen window has a right to handle all key events including
    // the preferred ones. Ash would offer the rest to the window first and
    // process them afterwards if it declined; without a key-ack extension
    // Lingmo has no afterwards, so everything else is processed now.
    wm::Window *active = m_windowManager ? m_windowManager->activation()->activeWindow() : nullptr;
    const bool fullscreen = active && active->state()->isFullscreen();
    return !fullscreen;
}

} // namespace Lingmo::Accelerators

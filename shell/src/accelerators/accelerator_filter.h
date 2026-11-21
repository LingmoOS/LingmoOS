// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/accelerator_filter.h and
//           chromium/src/ash/accelerators/pre_target_accelerator_handler.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the pre-target handler is an input::EventSink that
// sits in front of the window manager's, the way DemoController does. It also
// stands in for the aura::Env pre-target handlers Ash gives the accelerator
// history, the launcher state machine and the window cycle event filter.
#pragma once


#include "accelerators/accelerator.h"
#include "base/geometry.h"
#include "base/timer.h"
#include "input/event_sink.h"
#include "input/types.h"

namespace Lingmo::Input {
class XkbTracker;
}

namespace Lingmo::WM {
class WindowCycleController;
class WindowManager;
} // namespace Lingmo::WM

namespace Lingmo::Accelerators {

class AcceleratorController;

// ash::InputMethodManager::ArePositionalShortcutsUsedByCurrentInputMethod:
// every layout but the US ones pins punctuation shortcuts by position.
bool usesPositionalShortcuts(const input::XkbTracker &xkb);

class AcceleratorFilter : public input::EventSink
{

public:
    // `next` receives whatever is not consumed here; it is the window manager.
    AcceleratorFilter(AcceleratorController *controller, const input::XkbTracker *xkb,
                      wm::WindowManager *windowManager, wm::WindowCycleController *windowCycle,
                      input::EventSink *next);
    ~AcceleratorFilter() override;

    // The accelerator a physical key transition amounts to under the current
    // xkb state, which the seat has already advanced for it.
    Accelerator acceleratorForKey(uint32_t evdevCode, bool pressed) const;

    // libinput sends no repeats, so a held repeatable accelerator is performed
    // again at the keyboard's own rate, the way the seat repeats for clients.
    void setKeyRepeatSettings(const input::KeyRepeatSettings &settings);

    // input::EventSink:
    bool onPointerMotion(input::Timestamp timestamp, const base::PointF &location) override;
    bool onPointerButton(input::Timestamp timestamp, const base::PointF &location,
                         uint32_t evdevButton, bool pressed) override;
    bool onPointerAxis(input::Timestamp timestamp, const base::PointF &location,
                       const base::PointF &offset, bool discrete) override;
    bool onPointerAxisStop(input::Timestamp timestamp, const base::PointF &location) override;
    bool onKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed) override;
    bool onSecureKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed) override;
    // Touch passes straight through: ash's launcher state machine and window
    // cycle filter watch the mouse only.
    bool onTouchDown(input::Timestamp timestamp, int32_t id, const base::PointF &location) override;
    bool onTouchMotion(input::Timestamp timestamp, int32_t id, const base::PointF &location) override;
    bool onTouchUp(input::Timestamp timestamp, int32_t id) override;
    void onTouchCancel(input::Timestamp timestamp) override;

private:
    // PreTargetAcceleratorHandler::ProcessAccelerator.
    bool processAccelerator(const Accelerator &accelerator);
    bool shouldProcessAcceleratorNow(const Accelerator &accelerator) const;
    bool canConsumeSystemKeys() const;

    void startRepeat(const Accelerator &accelerator);
    void stopRepeat();
    void onRepeat();

    AcceleratorController *m_controller = nullptr;
    const input::XkbTracker *m_xkb = nullptr;
    // What the accelerator that started the window cycle held besides Shift.
    uint32_t m_cycleModifiers = 0;
    wm::WindowManager *m_windowManager = nullptr;
    wm::WindowCycleController *m_windowCycle = nullptr;
    input::EventSink *m_next = nullptr;

    input::KeyRepeatSettings m_repeatSettings;
    base::Timer m_repeatTimer;
    Accelerator m_repeatAccelerator;
};

} // namespace Lingmo::Accelerators


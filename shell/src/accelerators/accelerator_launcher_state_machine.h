// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/accelerator_launcher_state_machine.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: fed by the accelerator filter with keysyms and a
// pressed-key count instead of ui::KeyEvent and ui::InputController.
#pragma once


#include <cstdint>
#include <functional>

namespace Lingmo::Accelerators {

// Decides when releasing Meta may open the launcher: only when Meta went down
// with nothing else pressed and came back up with nothing else having been
// pressed or clicked in between.
class AcceleratorLauncherStateMachine
{
public:
    enum class LauncherState {
        // Waiting for the first input.
        Start,
        // The launcher may be triggered. In any other state it is suppressed.
        Trigger,
        // Waiting solely for the release of Meta to move to Trigger.
        Primed,
        // Suppressed until every key is released again.
        Suppress,
    };

    // `anyKeysPressed` answers whether any key is currently held.
    explicit AcceleratorLauncherStateMachine(std::function<bool()> anyKeysPressed);
    AcceleratorLauncherStateMachine(const AcceleratorLauncherStateMachine &) = delete;
    AcceleratorLauncherStateMachine &operator=(const AcceleratorLauncherStateMachine &) = delete;
    ~AcceleratorLauncherStateMachine();

    void onKey(uint32_t keysym, bool pressed);
    void onPointerButton(bool pressed);

    bool canHandleLauncher() const { return m_state == LauncherState::Trigger; }

    LauncherState currentState() const { return m_state; }

private:
    LauncherState m_state = LauncherState::Start;
    std::function<bool()> m_anyKeysPressed;
};

} // namespace Lingmo::Accelerators


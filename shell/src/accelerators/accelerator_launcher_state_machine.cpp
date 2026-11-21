// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/accelerator_launcher_state_machine.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_launcher_state_machine.h"

#include <utility>

#include <xkbcommon/xkbcommon-keysyms.h>

namespace Lingmo::Accelerators {

namespace {

bool isMetaKey(uint32_t keysym)
{
    return keysym == XKB_KEY_Super_L || keysym == XKB_KEY_Super_R;
}

bool isShiftKey(uint32_t keysym)
{
    return keysym == XKB_KEY_Shift_L || keysym == XKB_KEY_Shift_R;
}

} // namespace

AcceleratorLauncherStateMachine::AcceleratorLauncherStateMachine(
        std::function<bool()> anyKeysPressed)
    : m_anyKeysPressed(std::move(anyKeysPressed))
{
}

AcceleratorLauncherStateMachine::~AcceleratorLauncherStateMachine() = default;

void AcceleratorLauncherStateMachine::onKey(uint32_t keysym, bool pressed)
{
    switch (m_state) {
    // Trigger and Start share the same transitions: they differ only in
    // whether the launcher may open right now.
    case LauncherState::Trigger:
    case LauncherState::Start:
        if (!pressed || isShiftKey(keysym)) {
            m_state = LauncherState::Start;
            break;
        }
        m_state = isMetaKey(keysym) ? LauncherState::Primed : LauncherState::Suppress;
        break;

    // Anything but Meta moves to Suppress; Meta released moves to Trigger.
    case LauncherState::Primed:
        if (!isMetaKey(keysym)) {
            m_state = LauncherState::Suppress;
            break;
        }
        if (!pressed)
            m_state = LauncherState::Trigger;
        break;

    // Back to Start once no key is held any more.
    case LauncherState::Suppress:
        if (!m_anyKeysPressed())
            m_state = LauncherState::Start;
        break;
    }
}

void AcceleratorLauncherStateMachine::onPointerButton(bool pressed)
{
    switch (m_state) {
    case LauncherState::Start:
    case LauncherState::Trigger:
    case LauncherState::Primed:
        if (pressed)
            m_state = LauncherState::Suppress;
        break;
    case LauncherState::Suppress:
        if (!pressed && !m_anyKeysPressed())
            m_state = LauncherState::Start;
        break;
    }
}

} // namespace Lingmo::Accelerators

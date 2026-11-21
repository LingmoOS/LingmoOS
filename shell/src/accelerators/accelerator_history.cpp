// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/accelerator_history_impl.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "accelerators/accelerator_history.h"

namespace Lingmo::Accelerators {

AcceleratorHistory::AcceleratorHistory() = default;

AcceleratorHistory::~AcceleratorHistory() = default;

void AcceleratorHistory::storeCurrentAccelerator(const Accelerator &accelerator)
{
    // Track the pressed keys so that a key still held after another key was
    // released is not stored as a fresh press: holding Alt+Meta, releasing Alt
    // and keeping Meta down must not record a new Meta press. crbug.com/704280.
    if (accelerator.keyState() == Accelerator::KeyState::Pressed) {
        if (!m_currentlyPressedKeys.emplace(accelerator.keysym()).second)
            return;
    } else if (!m_currentlyPressedKeys.erase(accelerator.keysym())) {
        // A release with no press on record: the layout changed in between,
        // or the press was lost. Start over rather than leave keys stuck.
        m_currentlyPressedKeys.clear();
    }

    if (accelerator != m_currentAccelerator) {
        m_previousAccelerator = m_currentAccelerator;
        m_currentAccelerator = accelerator;
    }
}

} // namespace Lingmo::Accelerators

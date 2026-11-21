// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/accelerators/accelerator_history_impl.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: fed by the accelerator filter instead of being an
// aura pre-target handler.
#pragma once


#include <set>

#include "accelerators/accelerator.h"

namespace Lingmo::Accelerators {

// Keeps track of the system-wide current and previous key accelerators.
class AcceleratorHistory
{
public:
    AcceleratorHistory();
    AcceleratorHistory(const AcceleratorHistory &) = delete;
    AcceleratorHistory &operator=(const AcceleratorHistory &) = delete;
    ~AcceleratorHistory();

    const Accelerator &currentAccelerator() const { return m_currentAccelerator; }

    // The most recent accelerator that differs from the current one.
    const Accelerator &previousAccelerator() const { return m_previousAccelerator; }

    const std::set<uint32_t> &currentlyPressedKeys() const { return m_currentlyPressedKeys; }

    // Stores `accelerator` if it differs from the current one.
    void storeCurrentAccelerator(const Accelerator &accelerator);

private:
    Accelerator m_currentAccelerator;
    Accelerator m_previousAccelerator;
    std::set<uint32_t> m_currentlyPressedKeys;
};

} // namespace Lingmo::Accelerators


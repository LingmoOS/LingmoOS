// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/test/fake_display_delegate.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: callbacks and snapshots use the native display
// contract without Chromium task runners.

#include "fake_native_display_delegate.h"

namespace Lingmo::Test {

void FakeNativeDisplayDelegate::queryHardware(QueryCallback callback)
{
    ++m_queryCount;
    if (!m_snapshot.isValid()) {
        callback(std::nullopt,
                 {display::DisplayErrorCode::Unavailable,
                  "No valid hardware snapshot is available", {}});
        return;
    }
    callback(m_snapshot, {});
}

void FakeNativeDisplayDelegate::configure(const display::DisplayConfiguration& configuration,
                                          ConfigureMode mode, ConfigureCallback callback)
{
    ++m_configureCount;
    m_lastConfiguration = configuration;
    m_lastConfigureMode = mode;
    if (m_nextResult) {
        callback(std::move(*m_nextResult));
        m_nextResult.reset();
        return;
    }
    callback({true, false, {}, std::nullopt});
}

void FakeNativeDisplayDelegate::setDisplayPower(const std::vector<display::DisplayId>& displays,
                                                PowerState state, ConfigureCallback callback)
{
    m_lastPowerDisplays = displays;
    m_lastPowerState = state;
    callback({true, false, {}, std::nullopt});
}

} // namespace Lingmo::Test

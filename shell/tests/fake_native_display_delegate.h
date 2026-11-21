// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/test/fake_display_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: callbacks and snapshots use the native display
// contract without Chromium task runners.

#pragma once

#include <optional>

#include "display/native_display_delegate.h"

namespace Lingmo::Test {

class FakeNativeDisplayDelegate : public display::NativeDisplayDelegate
{
public:
    using NativeDisplayDelegate::NativeDisplayDelegate;

    void setSnapshot(display::HardwareSnapshot snapshot) { m_snapshot = std::move(snapshot); }
    void setNextResult(display::ConfigurationResult result) { m_nextResult = std::move(result); }

    int queryCount() const { return m_queryCount; }
    int configureCount() const { return m_configureCount; }
    std::optional<display::DisplayConfiguration> lastConfiguration() const
    {
        return m_lastConfiguration;
    }
    ConfigureMode lastConfigureMode() const { return m_lastConfigureMode; }

    void invalidate(uint64_t generation) { hardwareInvalidated(generation); }

    void takeDisplayControl(ControlCallback callback) override { callback(true); }
    void queryHardware(QueryCallback callback) override;
    void configure(const display::DisplayConfiguration& configuration, ConfigureMode mode,
                   ConfigureCallback callback) override;
    void setDisplayPower(const std::vector<display::DisplayId>& displays, PowerState state,
                         ConfigureCallback callback) override;

private:
    display::HardwareSnapshot m_snapshot;
    std::optional<display::ConfigurationResult> m_nextResult;
    std::optional<display::DisplayConfiguration> m_lastConfiguration;
    std::vector<display::DisplayId> m_lastPowerDisplays;
    PowerState m_lastPowerState = PowerState::On;
    ConfigureMode m_lastConfigureMode = ConfigureMode::TestOnly;
    int m_queryCount = 0;
    int m_configureCount = 0;
};

} // namespace Lingmo::Test

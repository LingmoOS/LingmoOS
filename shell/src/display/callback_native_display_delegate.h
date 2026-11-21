// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/types/native_display_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: injected callbacks connect the backend-independent
// native delegate contract to the compositor's in-process display runtime.

#pragma once

#include <memory>

#include "display/native_display_delegate.h"

namespace Lingmo::Display {

class CallbackNativeDisplayDelegate : public NativeDisplayDelegate
{
public:
    using ConfigureHandler =
            std::function<ConfigurationResult(const DisplayConfiguration &, ConfigureMode)>;
    using PowerHandler =
            std::function<ConfigurationResult(const std::vector<DisplayId> &, PowerState)>;

    CallbackNativeDisplayDelegate(HardwareSnapshot snapshot, ConfigureHandler configure,
                                  PowerHandler power);

    void invalidateSnapshots();
    void setSnapshot(HardwareSnapshot snapshot);
    void takeDisplayControl(ControlCallback callback) override;
    void queryHardware(QueryCallback callback) override;
    void configure(const DisplayConfiguration &configuration, ConfigureMode mode,
                   ConfigureCallback callback) override;
    void setDisplayPower(const std::vector<DisplayId> &displays, PowerState state,
                         ConfigureCallback callback) override;

private:
    uint64_t m_epoch = 0;
    // Results are delivered from the loop; they are dropped once this goes.
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    HardwareSnapshot m_snapshot;
    ConfigureHandler m_configure;
    PowerHandler m_power;
};

} // namespace Lingmo::Display

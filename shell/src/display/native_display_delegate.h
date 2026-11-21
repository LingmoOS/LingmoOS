// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/types/native_display_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: callbacks use standard C++ values and expose the
// backend boundary needed by a single-process compositor.

#pragma once

#include <functional>

#include "base/signal.h"
#include "display/display_types.h"

namespace Lingmo::Display {

class NativeDisplayDelegate
{
public:
    enum class ConfigureMode {
        TestOnly,
        Commit,
    };

    enum class PowerState {
        On,
        Off,
        Suspend,
    };

    using ControlCallback = std::function<void(bool)>;
    using QueryCallback = std::function<void(std::optional<HardwareSnapshot>, DisplayError)>;
    using ConfigureCallback = std::function<void(ConfigurationResult)>;

    NativeDisplayDelegate() = default;
    virtual ~NativeDisplayDelegate() = default;

    NativeDisplayDelegate(const NativeDisplayDelegate&) = delete;
    NativeDisplayDelegate& operator=(const NativeDisplayDelegate&) = delete;

    virtual void takeDisplayControl(ControlCallback callback) = 0;
    virtual void queryHardware(QueryCallback callback) = 0;
    virtual void configure(const DisplayConfiguration& configuration, ConfigureMode mode,
                           ConfigureCallback callback) = 0;
    virtual void setDisplayPower(const std::vector<DisplayId> & displays, PowerState state,
                                 ConfigureCallback callback) = 0;

    // The topology generation the next snapshot will carry.
    base::Signal<uint64_t> hardwareInvalidated;
};

} // namespace Lingmo::Display

// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/display_configurator.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: a serialized callback pipeline validates, tests,
// and commits one revisioned configuration through NativeDisplayDelegate.

#pragma once

#include <functional>
#include <memory>

#include "base/signal.h"

#include "display/display_configuration_validator.h"
#include "display/native_display_delegate.h"

namespace Lingmo::Display {

class DisplayConfigurator
{
public:
    using Completion = std::function<void(ConfigurationResult)>;

    explicit DisplayConfigurator(NativeDisplayDelegate* delegate);
    DisplayConfigurator(const DisplayConfigurator&) = delete;
    DisplayConfigurator& operator=(const DisplayConfigurator&) = delete;

    void test(const HardwareSnapshot& hardware, uint64_t revision,
              const DisplayConfiguration& request, Completion completion);
    void apply(const HardwareSnapshot& hardware, uint64_t revision,
               const DisplayConfiguration& request, Completion completion);

private:
    void run(const HardwareSnapshot& hardware, uint64_t revision,
             const DisplayConfiguration& request, bool commit, Completion completion);

    NativeDisplayDelegate* m_delegate = nullptr;
    bool m_busy = false;
    bool m_invalidated = false;
    base::Subscription m_hardwareInvalidated;
    // Completions arriving after this goes are dropped.
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Display

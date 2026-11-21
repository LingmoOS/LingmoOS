// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/types/native_display_delegate.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: queue native operations on the event loop while
// keeping the backend-independent completion interface.

#include "display/callback_native_display_delegate.h"

#include "base/event_loop.h"
#include "core/main_loop.h"

namespace Lingmo::Display {

CallbackNativeDisplayDelegate::CallbackNativeDisplayDelegate(HardwareSnapshot snapshot,
                                                             ConfigureHandler configure,
                                                             PowerHandler power)
    : m_snapshot(std::move(snapshot))
    , m_configure(std::move(configure))
    , m_power(std::move(power))
{
}

void CallbackNativeDisplayDelegate::invalidateSnapshots()
{
    ++m_epoch;
    hardwareInvalidated(m_snapshot.topologyGeneration + 1);
}

void CallbackNativeDisplayDelegate::setSnapshot(HardwareSnapshot snapshot)
{
    ++m_epoch;
    const uint64_t generation = snapshot.topologyGeneration;
    m_snapshot = std::move(snapshot);
    hardwareInvalidated(generation);
}

void CallbackNativeDisplayDelegate::takeDisplayControl(ControlCallback callback)
{
    callback(true);
}

void CallbackNativeDisplayDelegate::queryHardware(QueryCallback callback)
{
    if (m_snapshot.isValid())
        callback(m_snapshot, {});
    else
        callback(std::nullopt,
                 {DisplayErrorCode::Unavailable,
                  "Display hardware is unavailable",
                  {}});
}

void CallbackNativeDisplayDelegate::configure(const DisplayConfiguration &configuration,
                                              ConfigureMode mode, ConfigureCallback callback)
{
    core::mainLoop()->post(
            [this, alive = std::weak_ptr<bool>(m_alive), configuration, mode, epoch = m_epoch,
             callback = std::move(callback)]() mutable {
                if (alive.expired())
                    return;
                if (epoch != m_epoch) {
                    callback({false,
                              false,
                              {DisplayErrorCode::StaleConfiguration,
                               "Display snapshots were invalidated",
                               {}},
                              std::nullopt});
                    return;
                }
                callback(m_configure ? m_configure(configuration, mode)
                                     : ConfigurationResult {
                                               false,
                                               false,
                                               {DisplayErrorCode::Unavailable,
                                                "No display backend is available",
                                                {}},
                                               std::nullopt});
            });
}

void CallbackNativeDisplayDelegate::setDisplayPower(const std::vector<DisplayId> &displays,
                                                    PowerState state, ConfigureCallback callback)
{
    core::mainLoop()->post(
            [this, alive = std::weak_ptr<bool>(m_alive), displays, state, epoch = m_epoch,
             callback = std::move(callback)]() mutable {
                if (alive.expired())
                    return;
                if (epoch != m_epoch) {
                    callback({false,
                              false,
                              {DisplayErrorCode::StaleConfiguration,
                               "Display snapshots were invalidated",
                               {}},
                              std::nullopt});
                    return;
                }
                callback(m_power ? m_power(displays, state)
                                 : ConfigurationResult {
                                           false,
                                           false,
                                           {DisplayErrorCode::UnsupportedProperty,
                                            "Display power control is unavailable",
                                            {}},
                                           std::nullopt});
            });
}

} // namespace Lingmo::Display

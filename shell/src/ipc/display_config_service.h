// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/cros_display_config.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: a credential-checked session D-Bus object fronts the
// compositor transaction controller without Ash or Mojo.

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "display/display_configuration_controller.h"
#include "ipc/display_dbus_types.h"

namespace Lingmo::Display::ipc {

// org.lingmo.DisplayConfig1 at /org/lingmo/DisplayConfig1.
class DisplayConfigService
{
public:
    using SessionActive = std::function<bool()>;

    DisplayConfigService(DisplayManager* manager, DisplayConfigurationController* controller,
                         base::dbus::Connection* connection,
                         SessionActive sessionActive);
    ~DisplayConfigService();

    DisplayConfigService(const DisplayConfigService&) = delete;
    DisplayConfigService& operator=(const DisplayConfigService&) = delete;

    // Publishes the object, then takes the bus name.
    bool registerService(const std::string& serviceName);

    // Receives SetNightLight's strength, 0 to 1, and how long to take to it.
    using NightLightHandler = std::function<void(double, std::chrono::milliseconds)>;

    void setNightLightHandler(NightLightHandler handler) { m_nightLight = std::move(handler); }

private:
    void addMethods();
    // Both reply with the error themselves when they refuse.
    bool authorizeWrite(base::dbus::MethodCall& call);
    // The caller owns this session, whether or not it is on screen.
    bool authorizeCaller(base::dbus::MethodCall& call);
    std::string reject(DisplayErrorCode code, const std::string& message);
    uint64_t topologyGeneration() const;
    uint64_t appliedRevision() const;

    DisplayManager* m_manager = nullptr;
    DisplayConfigurationController* m_controller = nullptr;
    base::dbus::Connection* m_connection = nullptr;
    SessionActive m_sessionActive;
    NightLightHandler m_nightLight;
    std::string m_serviceName;
    std::unique_ptr<base::dbus::Object> m_object;
    base::dbus::Match m_ownerChanges;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Display::ipc

// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/cros_display_config.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: typed D-Bus calls are authorized by peer credentials
// and active-seat state before entering the shared transaction authority.

#include "ipc/display_config_service.h"

#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <tuple>

#include "base/uuid.h"
#include "display/recommended_scale.h"

namespace Lingmo::Display::ipc {

namespace dbus = base::dbus;

static constexpr const char* kPath = "/org/lingmo/DisplayConfig1";
static constexpr const char* kInterface = "org.lingmo.DisplayConfig1";
static constexpr const char* kUnauthorized = "org.lingmo.DisplayConfig1.Error.Unauthorized";

DisplayConfigService::DisplayConfigService(DisplayManager* manager,
                                           DisplayConfigurationController* controller,
                                           dbus::Connection* connection,
                                           SessionActive sessionActive)
    : m_manager(manager)
    , m_controller(controller)
    , m_connection(connection)
    , m_sessionActive(std::move(sessionActive))
{
    if (m_connection) {
        m_object = std::make_unique<dbus::Object>(m_connection, kPath, kInterface);
        addMethods();
    }
    m_subscriptions.push_back(m_manager->displaysChanged.connect(
            [this](uint64_t generation, uint64_t revision) {
                if (m_object)
                    m_object->emitSignal("StateChanged", generation, revision);
            }));
    m_subscriptions.push_back(m_controller->previewChanged.connect(
            [this](const std::string& token, int64_t deadline) {
                if (m_object)
                    m_object->emitSignal("PreviewChanged", token, uint64_t(deadline),
                                         topologyGeneration(), appliedRevision());
            }));
    m_subscriptions.push_back(m_controller->operationFinished.connect(
            [this](const std::string& token, const ConfigurationResult& result) {
                std::vector<std::string> affected;
                for (const auto& id : result.error.affectedDisplays)
                    affected.push_back(id.value());
                if (m_object)
                    m_object->emitSignal("OperationFinished", token, result.succeeded,
                                         result.degraded, uint32_t(result.error.code),
                                         result.error.message, affected, topologyGeneration(),
                                         appliedRevision());
            }));
    if (m_connection) {
        dbus::MatchRule rule;
        rule.sender = "org.freedesktop.DBus";
        rule.path = "/org/freedesktop/DBus";
        rule.interface = "org.freedesktop.DBus";
        rule.member = "NameOwnerChanged";
        m_ownerChanges = m_connection->subscribe(rule, [this](const dbus::Message& message) {
            std::string name;
            std::string oldOwner;
            std::string newOwner;
            if (message.read(name, oldOwner, newOwner) && !oldOwner.empty() && newOwner.empty())
                m_controller->ownerDisconnected(name);
        });
    }
}

DisplayConfigService::~DisplayConfigService()
{
    if (m_connection && !m_serviceName.empty())
        m_connection->releaseName(m_serviceName);
}

bool DisplayConfigService::registerService(const std::string& serviceName)
{
    if (!m_connection || !m_object || serviceName.empty())
        return false;
    m_serviceName = serviceName;
    return m_object->publish() && m_connection->requestName(serviceName);
}

uint64_t DisplayConfigService::topologyGeneration() const
{
    return m_manager->hardware() ? m_manager->hardware()->topologyGeneration : 0;
}

uint64_t DisplayConfigService::appliedRevision() const
{
    return m_manager->applied() ? m_manager->applied()->revision : 0;
}

void DisplayConfigService::addMethods()
{
    m_object->addMethod("GetState", [this] {
        const auto hardware = m_manager->hardware();
        const auto applied = m_manager->applied();
        const auto& physical = applied ? applied->physicalDisplays
                                       : (hardware ? hardware->displays
                                                   : std::vector<DisplaySnapshot>());
        return std::tuple(
                hardware ? hardware->compositorInstanceId : std::string(),
                hardware ? hardware->seatId : std::string(), topologyGeneration(),
                appliedRevision(), toWire(physical),
                toWire(applied ? applied->logicalDisplays : std::vector<Lingmo::Display::LogicalDisplay>()),
                toWire(applied ? applied->placements : std::vector<DisplayPlacement>()),
                m_controller->previewToken(), uint64_t(m_controller->previewDeadlineMs()));
    }, dbus::ArgumentNames {{},
                            {"instanceId", "seatId", "topologyGeneration", "appliedRevision",
                             "physicalDisplays", "logicalDisplays", "placements", "previewToken",
                             "previewDeadlineMs"}});

    using Configuration = std::tuple<std::string, uint64_t, uint64_t, std::vector<DisplayRequest>,
                                     std::string, std::vector<Placement>, MirrorGroups>;
    const std::string configurationIn = dbus::Type<Configuration>::signature();
    // Without the parentheses of the struct: the arguments are separate.
    const std::string configurationArguments =
            configurationIn.substr(1, configurationIn.size() - 2);
    const auto readConfiguration = [](dbus::MethodCall& call) -> std::optional<DisplayConfiguration> {
        std::string instanceId;
        uint64_t generation = 0;
        uint64_t revision = 0;
        std::vector<DisplayRequest> displays;
        std::string primary;
        std::vector<Placement> placements;
        MirrorGroups mirrorGroups;
        if (!call.read(instanceId, generation, revision, displays, primary, placements,
                       mirrorGroups)) {
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
            return std::nullopt;
        }
        return fromWire(instanceId, generation, revision, displays, primary, placements,
                        mirrorGroups);
    };

    const dbus::ArgumentNames configurationNames {
            {"instanceId", "topologyGeneration", "appliedRevision", "displays", "primary",
             "placements", "mirrorGroups"},
            {"operationToken"}};
    m_object->addMethod("TestConfiguration", configurationArguments, "s",
                        [this, readConfiguration](dbus::MethodCall& call) {
                            if (!authorizeWrite(call))
                                return;
                            if (const auto request = readConfiguration(call))
                                call.reply(m_controller->test(*request));
                        }, configurationNames);
    m_object->addMethod("ApplyConfiguration", configurationArguments, "s",
                        [this, readConfiguration](dbus::MethodCall& call) {
                            if (!authorizeWrite(call))
                                return;
                            if (const auto request = readConfiguration(call))
                                call.reply(m_controller->apply(*request,
                                                               call.message().sender()));
                        }, configurationNames);
    m_object->addMethod("SetConfiguration", configurationArguments, "s",
                        [this, readConfiguration](dbus::MethodCall& call) {
                            if (!authorizeWrite(call))
                                return;
                            if (const auto request = readConfiguration(call))
                                call.reply(m_controller->apply(*request, call.message().sender(),
                                                               0));
                        }, configurationNames);

    m_object->addMethod("ConfirmConfiguration", "s", "", [this](dbus::MethodCall& call) {
        std::string token;
        call.read(token);
        if (!authorizeWrite(call))
            return;
        m_controller->confirm(token, call.message().sender());
        call.reply();
    }, dbus::ArgumentNames {{"previewToken"}, {}});
    m_object->addMethod("RevertConfiguration", "s", "", [this](dbus::MethodCall& call) {
        std::string token;
        call.read(token);
        if (!authorizeWrite(call))
            return;
        m_controller->revert(token, call.message().sender());
        call.reply();
    }, dbus::ArgumentNames {{"previewToken"}, {}});

    m_object->addMethod("SetDisplayPower", "asu", "s", [this](dbus::MethodCall& call) {
        std::vector<std::string> displayIds;
        uint32_t state = 0;
        call.read(displayIds, state);
        if (!authorizeWrite(call))
            return;
        if (state > uint32_t(NativeDisplayDelegate::PowerState::Suspend)) {
            call.reply(reject(DisplayErrorCode::UnsupportedProperty,
                              "The display power state is invalid"));
            return;
        }
        std::vector<DisplayId> ids;
        for (const auto& value : displayIds)
            ids.push_back(DisplayId(value));
        call.reply(m_controller->setPower(ids, NativeDisplayDelegate::PowerState(state)));
    }, dbus::ArgumentNames {{"displayIds", "state"}, {"operationToken"}});

    m_object->addMethod("SetDockedLidClosed", "b", "s", [this](dbus::MethodCall& call) {
        bool closed = false;
        call.read(closed);
        if (!authorizeWrite(call))
            return;
        if (!m_manager->hardware()) {
            call.reply(reject(DisplayErrorCode::Unavailable, "Display hardware is unavailable"));
            return;
        }
        // Between a hotplug and its new layout nothing is applied, which is not
        // "no external display".
        if (closed && !m_manager->applied()) {
            call.reply(reject(DisplayErrorCode::Busy, "A display change is in progress"));
            return;
        }
        bool usableExternal = false;
        std::vector<DisplayId> internal;
        for (const auto& physical : m_manager->hardware()->displays) {
            if (physical.connectionType == DisplayConnectionType::Internal)
                internal.push_back(physical.sessionId);
            else if (closed && m_manager->applied()) {
                for (const auto& logical : m_manager->applied()->logicalDisplays)
                    usableExternal |= logical.hasDestination(physical.sessionId);
            }
        }
        if (closed && !usableExternal) {
            call.reply(reject(DisplayErrorCode::NoEnabledOutput,
                              "No active external display is available for docked mode"));
            return;
        }
        call.reply(m_controller->setPower(internal, closed ? NativeDisplayDelegate::PowerState::Off
                                                           : NativeDisplayDelegate::PowerState::On));
    }, dbus::ArgumentNames {{"closed"}, {"operationToken"}});

    m_object->addMethod("SetNightLight", "du", "", [this](dbus::MethodCall& call) {
        double strength = 0;
        uint32_t durationMs = 0;
        call.read(strength, durationMs);
        // A tint, not a configuration: a session that is not on screen keeps it
        // for when it is back.
        if (!authorizeCaller(call))
            return;
        if (!std::isfinite(strength)) {
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs",
                            "The strength is not a number");
            return;
        }
        if (m_nightLight)
            m_nightLight(std::clamp(strength, 0.0, 1.0), std::chrono::milliseconds(durationMs));
        call.reply();
    }, dbus::ArgumentNames {{"strength", "durationMs"}, {}});

    m_object->addSignal("StateChanged", "tt", {"topologyGeneration", "appliedRevision"});
    m_object->addSignal("OperationFinished", "sbbusastt",
                        {"operationToken", "succeeded", "degraded", "errorCode", "errorMessage",
                         "affectedDisplayIds", "topologyGeneration", "appliedRevision"});
    m_object->addSignal("PreviewChanged", "sttt",
                        {"previewToken", "deadlineMs", "topologyGeneration", "appliedRevision"});
}

bool DisplayConfigService::authorizeWrite(dbus::MethodCall& call)
{
    if (!m_sessionActive()) {
        call.replyError(kUnauthorized, "The graphical session is inactive");
        return false;
    }
    return authorizeCaller(call);
}

bool DisplayConfigService::authorizeCaller(dbus::MethodCall& call)
{
    dbus::Message query = m_connection->newMethodCall("org.freedesktop.DBus",
                                                      "/org/freedesktop/DBus",
                                                      "org.freedesktop.DBus",
                                                      "GetConnectionUnixUser");
    query.append(call.message().sender());
    uint32_t uid = 0;
    if (!m_connection->call(query).read(uid) || uid != uint32_t(getuid())) {
        call.replyError(kUnauthorized, "The caller does not own this user session");
        return false;
    }
    return true;
}

std::string DisplayConfigService::reject(DisplayErrorCode code, const std::string& message)
{
    const std::string token = "rejected-" + base::Uuid::generate().toString();
    if (m_object)
        m_object->emitSignal("OperationFinished", token, false, false, uint32_t(code), message,
                             std::vector<std::string>(), topologyGeneration(), appliedRevision());
    return token;
}

} // namespace Lingmo::Display::ipc

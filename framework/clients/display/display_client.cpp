// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Upstream: chromium/src/ash/display/cros_display_config.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Cutefish: asynchronous D-Bus calls, service-owner watching and
// reconnecting after compositor restarts.

#include "lingmo/display/display_client.h"

#include <algorithm>
#include <chrono>
#include <utility>

#include "framework_i18n.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;
using Lingmo::framework::tr;

namespace Lingmo::Display {

static constexpr const char* kPath = "/org/lingmo/DisplayConfig1";
static constexpr const char* kInterface = "org.lingmo.DisplayConfig1";

DisplayClient::DisplayClient(std::string serviceName)
    : m_serviceName(std::move(serviceName))
    , m_proxy(dbus::Connection::session(), m_serviceName, kPath, kInterface)
{
    m_operationTimeout.setSingleShot(true);
    m_operationTimeout.setInterval(15s);
    m_operationTimeout.onTimeout = [this] {
        finishOperation(m_operationToken, {false, false, 1, tr("The display operation timed out."), {}});
        refresh();
    };
    dbus::Connection* bus = m_proxy.connection();
    if (!bus) {
        m_loaded = true;
        return;
    }
    m_matches.push_back(m_proxy.onSignal("StateChanged", [this](const dbus::Message&) { refresh(); }));
    m_matches.push_back(m_proxy.onSignal("PreviewChanged", [this](const dbus::Message&) { refresh(); }));
    m_matches.push_back(m_proxy.onSignal("OperationFinished", [this](const dbus::Message& message) {
        operationFinishedSignal(message);
    }));
    // Looked up at once, so a request made right after construction reaches a running compositor.
    const dbus::Reply owner = dbus::Proxy(bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus")
                                  .call("GetNameOwner", m_serviceName);
    m_owner = owner.value<std::string>().value_or(std::string());
    m_watch = bus->watchName(m_serviceName, [this](const std::string& newOwner) {
        if (newOwner == m_owner)
            return;
        m_owner = newOwner;
        const bool wasAvailable = available();
        serviceChanged();
        if (wasAvailable != available())
            availabilityChanged.emit();
        refresh();
    });
    serviceChanged();
    refresh();
}

DisplayClient::~DisplayClient() = default;

bool DisplayClient::available() const
{
    return hasService() && !m_state.instanceId.empty();
}

void DisplayClient::serviceChanged()
{
    ++m_refreshSerial;
    ++m_serviceEpoch;
    m_refreshCall.reset();
    m_state = {};
    m_loaded = false;
    stateChanged.emit();
    if (operationPending())
        finishOperation(m_operationToken, {false, false, 1, tr("The display service restarted."), {}});
    // Last, so a completion that asks again reaches the new owner.
    failRequests();
}

void DisplayClient::operationFinishedSignal(const dbus::Message& message)
{
    std::string token;
    OperationResult result;
    if (!message.read(token, result.succeeded, result.degraded, result.errorCode, result.message, result.affected,
                      result.generation, result.revision))
        return;
    if (const auto request = m_requests.find(token); request != m_requests.end()) {
        const Completion done = std::move(request->second);
        m_requests.erase(request);
        done(result.succeeded, ErrorCode(result.errorCode));
    } else if (m_requestsInFlight) {
        m_earlyRequestResults.insert_or_assign(token, result);
    }
    if (m_applyPending)
        m_earlyResults.insert_or_assign(token, result);
    else if (token == m_operationToken)
        queueCompletion(token, result);
    refresh();
}

void DisplayClient::refresh()
{
    const uint64_t serial = ++m_refreshSerial;
    if (!hasService()) {
        m_refreshCall.reset();
        m_state = {};
        m_loaded = true;
        stateChanged.emit();
        return;
    }
    m_refreshCall = m_proxy.callAsync("GetState", [this, serial](const dbus::Reply& reply) {
        if (serial != m_refreshSerial)
            return;
        m_loaded = true;
        State state;
        if (!reply.read(state.instanceId, state.seatId, state.topologyGeneration, state.appliedRevision,
                        state.physical, state.logical, state.placements, state.previewToken,
                        state.previewDeadlineMs)) {
            m_state = {};
            stateChanged.emit();
            availabilityChanged.emit();
            if (operationPending()) {
                const std::string message = reply.isError() ? reply.error().message : tr("Unexpected display state.");
                finishOperation(m_operationToken, {false, false, 1, message, {}});
            }
            return;
        }
        const bool wasAvailable = available();
        m_state = std::move(state);
        stateChanged.emit();
        if (wasAvailable != available())
            availabilityChanged.emit();
        completeIfCurrent();
    });
}

bool DisplayClient::apply(const std::vector<DisplayRequest>& displays, const std::string& primary,
                          const std::vector<Placement>& placements, const MirrorGroups& mirrorGroups, bool preview)
{
    if (!available() || operationPending())
        return false;
    m_applyPending = true;
    m_operationToken.clear();
    m_earlyResults.clear();
    m_operationTimeout.start();
    const uint64_t epoch = m_serviceEpoch;
    const uint64_t operation = ++m_operationSerial;
    const uint64_t id = ++m_nextCall;
    keepCall(id, m_proxy.callAsync(
                     preview ? "ApplyConfiguration" : "SetConfiguration",
                     [this, epoch, operation, id](const dbus::Reply& reply) {
                         m_calls.erase(id);
                         if (epoch != m_serviceEpoch || operation != m_operationSerial || !m_applyPending)
                             return;
                         m_applyPending = false;
                         std::string token;
                         if (!reply.read(token)) {
                             queueCompletion({}, {false, false, 1, reply.error().message, {}});
                             return;
                         }
                         m_operationToken = token;
                         if (const auto result = m_earlyResults.find(token); result != m_earlyResults.end())
                             queueCompletion(token, result->second);
                         m_earlyResults.clear();
                     },
                     m_state.instanceId, m_state.topologyGeneration, m_state.appliedRevision, displays, primary,
                     placements, mirrorGroups));
    return true;
}

void DisplayClient::finishOperation(const std::string& token, const OperationResult& result)
{
    const std::string completedToken = token;
    m_applyPending = false;
    m_operationToken.clear();
    m_earlyResults.clear();
    m_completion.reset();
    m_operationTimeout.stop();
    operationFinished.emit({completedToken, result.succeeded, result.degraded, result.errorCode, result.message,
                            result.affected});
}

bool DisplayClient::operationPending() const
{
    return m_applyPending || !m_operationToken.empty() || m_completion;
}

void DisplayClient::queueCompletion(const std::string& token, const OperationResult& result)
{
    m_operationToken = token;
    m_completion = result;
    refresh();
}

void DisplayClient::completeIfCurrent()
{
    if (!m_completion)
        return;
    if (m_completion->generation
        && (m_state.topologyGeneration < m_completion->generation
            || (m_state.topologyGeneration == m_completion->generation
                && m_state.appliedRevision < m_completion->revision)))
        return;
    const OperationResult result = *m_completion;
    finishOperation(m_operationToken, result);
}

void DisplayClient::confirm(const std::string& token)
{
    if (available())
        m_proxy.send("ConfirmConfiguration", token);
}

void DisplayClient::revert(const std::string& token)
{
    if (available())
        m_proxy.send("RevertConfiguration", token);
}

bool DisplayClient::setDisplayPower(bool on, Completion done)
{
    if (!hasService())
        return false;
    // The compositor's power states: 0 is on, 1 is off.
    dbus::Message call = m_proxy.connection()->newMethodCall(m_serviceName, kPath, kInterface, "SetDisplayPower");
    call.append(std::vector<std::string>(), uint32_t(on ? 0 : 1));
    return sendRequest(std::move(call), std::move(done));
}

bool DisplayClient::setDockedLidClosed(bool closed, Completion done)
{
    if (!hasService())
        return false;
    dbus::Message call = m_proxy.connection()->newMethodCall(m_serviceName, kPath, kInterface, "SetDockedLidClosed");
    call.append(closed);
    return sendRequest(std::move(call), std::move(done));
}

bool DisplayClient::setNightLight(double strength, uint32_t durationMs)
{
    if (!hasService())
        return false;
    m_proxy.send("SetNightLight", strength, durationMs);
    return true;
}

bool DisplayClient::nightLightAvailable() const
{
    return std::any_of(m_state.physical.begin(), m_state.physical.end(), [](const PhysicalDisplay& display) {
        return display.capabilities & ColorTransformCapability;
    });
}

bool DisplayClient::sendRequest(dbus::Message call, Completion done)
{
    ++m_requestsInFlight;
    const uint64_t epoch = m_serviceEpoch;
    const uint64_t id = ++m_nextCall;
    keepCall(id, m_proxy.connection()->callAsync(call, [this, epoch, id, done = std::move(done)](const dbus::Reply& reply) {
        m_calls.erase(id);
        // Sent to a previous owner, whose OperationFinished never reaches this client.
        if (epoch != m_serviceEpoch) {
            if (done)
                done(false, ErrorCode::Unavailable);
            return;
        }

        // A rejected request reports OperationFinished before its reply.
        std::string token;
        const bool replied = reply.read(token);
        std::optional<OperationResult> early;
        if (replied) {
            if (const auto result = m_earlyRequestResults.find(token); result != m_earlyRequestResults.end()) {
                early = result->second;
                m_earlyRequestResults.erase(result);
            }
        }
        if (--m_requestsInFlight == 0)
            m_earlyRequestResults.clear();

        if (!done)
            return;
        if (!replied)
            done(false, ErrorCode::Unavailable);
        else if (early)
            done(early->succeeded, ErrorCode(early->errorCode));
        else
            m_requests.insert_or_assign(token, done);
    }));
    return true;
}

void DisplayClient::keepCall(uint64_t id, dbus::PendingCall call)
{
    // A call that failed at once has already been answered, and its id erased.
    if (!call.isNull())
        m_calls.emplace(id, std::move(call));
}

void DisplayClient::failRequests()
{
    const auto requests = std::exchange(m_requests, {});
    m_requestsInFlight = 0;
    m_earlyRequestResults.clear();
    for (const auto& [token, done] : requests)
        done(false, ErrorCode::Unavailable);
}

} // namespace Lingmo::Display

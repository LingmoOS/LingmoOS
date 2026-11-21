// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Upstream: chromium/src/ash/display/cros_display_config.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Cutefish: an asynchronous D-Bus client watches compositor
// ownership and exposes coherent native display snapshots.

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/display/display_types.h"

namespace Lingmo::Display {

class DisplayClient
{
public:
    struct State
    {
        std::string instanceId;
        std::string seatId;
        uint64_t topologyGeneration = 0;
        uint64_t appliedRevision = 0;
        std::vector<PhysicalDisplay> physical;
        std::vector<LogicalDisplay> logical;
        std::vector<Placement> placements;
        std::string previewToken;
        uint64_t previewDeadlineMs = 0;
    };

    struct Operation
    {
        std::string token;
        bool succeeded = false;
        bool degraded = false;
        uint32_t errorCode = 0;
        std::string message;
        std::vector<std::string> affected;
    };

    // Called once with the compositor's verdict on a request.
    using Completion = std::function<void(bool succeeded, ErrorCode error)>;

    // Tests name a service of their own.
    explicit DisplayClient(std::string serviceName = "org.lingmo.DisplayConfig");
    ~DisplayClient();

    DisplayClient(const DisplayClient&) = delete;
    DisplayClient& operator=(const DisplayClient&) = delete;

    bool available() const;

    // False until the first GetState reply after (re)connecting.
    bool loaded() const { return m_loaded; }

    const State& state() const { return m_state; }

    void refresh();
    // A preview is applied but persisted only by confirm(); the compositor
    // reverts it on its own deadline otherwise. False when nothing was sent.
    bool apply(const std::vector<DisplayRequest>& displays, const std::string& primary,
               const std::vector<Placement>& placements, const MirrorGroups& mirrorGroups, bool preview = false);
    void confirm(const std::string& token);
    void revert(const std::string& token);

    // Powers every display on or off. False when the compositor is not on the bus,
    // and then `done` is never called.
    bool setDisplayPower(bool on, Completion done = {});

    // Closing fails with NoEnabledOutput when no external display is in use.
    bool setDockedLidClosed(bool closed, Completion done = {});

    // Moves the night light to `strength`, 0 for off to 1 for the warmest, over
    // `durationMs`. False when the compositor is not on the bus.
    bool setNightLight(double strength, uint32_t durationMs);

    // Whether a display in the current state can be tinted.
    bool nightLightAvailable() const;

    base::Signal<> availabilityChanged;
    base::Signal<> stateChanged;
    base::Signal<Operation> operationFinished;

private:
    struct OperationResult
    {
        bool succeeded = false;
        bool degraded = false;
        uint32_t errorCode = 0;
        std::string message;
        std::vector<std::string> affected;
        uint64_t generation = 0;
        uint64_t revision = 0;
    };

    bool hasService() const { return m_proxy.isValid() && !m_owner.empty(); }
    void serviceChanged();
    void operationFinishedSignal(const base::dbus::Message& message);
    bool sendRequest(base::dbus::Message call, Completion done);
    void failRequests();
    // An apply waits for its reply, its OperationFinished, or the state that shows it.
    bool operationPending() const;
    void queueCompletion(const std::string& token, const OperationResult& result);
    void completeIfCurrent();
    void finishOperation(const std::string& token, const OperationResult& result);
    // Keeps a reply's slot until the reply has been handled.
    void keepCall(uint64_t id, base::dbus::PendingCall call);

    std::string m_serviceName;
    base::dbus::Proxy m_proxy;
    std::string m_owner;
    base::dbus::NameWatch m_watch;
    std::vector<base::dbus::Match> m_matches;
    State m_state;
    bool m_loaded = false;
    uint64_t m_refreshSerial = 0;
    base::dbus::PendingCall m_refreshCall;
    bool m_applyPending = false;
    std::string m_operationToken;
    std::map<std::string, OperationResult> m_earlyResults;
    std::optional<OperationResult> m_completion;
    uint64_t m_serviceEpoch = 0;
    uint64_t m_operationSerial = 0;
    base::Timer m_operationTimeout;
    uint64_t m_nextCall = 0;
    std::map<uint64_t, base::dbus::PendingCall> m_calls;

    // Power and lid requests run beside apply and do not wait for a state refresh.
    int m_requestsInFlight = 0;
    std::map<std::string, Completion> m_requests;
    std::map<std::string, OperationResult> m_earlyRequestResults;
};

} // namespace Lingmo::Display

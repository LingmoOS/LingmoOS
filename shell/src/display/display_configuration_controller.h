// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/resolution_notification_controller.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: every user display change is a compositor-owned
// preview transaction that confirms, times out, or restores prior state.

#pragma once

#include <optional>

#include "base/signal.h"
#include "base/timer.h"

#include "display/display_configurator.h"
#include "display/display_manager.h"
#include "display/display_preferences.h"

namespace Lingmo::Display {

class DisplayConfigurationController
{
public:
    explicit DisplayConfigurationController(DisplayManager *manager,
                                            NativeDisplayDelegate *delegate,
                                            DisplayPreferences preferences = DisplayPreferences());
    DisplayConfigurationController(const DisplayConfigurationController &) = delete;
    DisplayConfigurationController &operator=(const DisplayConfigurationController &) = delete;

    std::string test(const DisplayConfiguration &request);
    // A zero timeout commits and persists immediately, without a preview.
    std::string apply(const DisplayConfiguration &request, const std::string &owner,
                      int timeoutMs = 15000);
    void confirm(const std::string &previewToken, const std::string &owner);
    void revert(const std::string &previewToken, const std::string &owner);
    std::string setPower(const std::vector<DisplayId> &displays,
                         NativeDisplayDelegate::PowerState state);
    void ownerDisconnected(const std::string &owner);
    void hardwareInvalidated(uint64_t generation);

    std::optional<AppliedConfiguration> recoveryState() const
    {
        return m_preview ? std::optional(m_preview->rollbackApplied) : m_manager->applied();
    }

    void setTopologyPending(bool pending) { m_topologyPending = pending; }

    bool busy() const { return m_operationActive; }

    std::string previewToken() const;

    int64_t previewDeadlineMs() const { return m_preview ? m_preview->deadlineMs : 0; }

    // The operation's token and its result.
    base::Signal<const std::string &, const ConfigurationResult &> operationFinished;
    // The preview's token and its deadline in milliseconds since the epoch;
    // an empty token when no preview is pending.
    base::Signal<const std::string &, int64_t> previewChanged;

private:
    struct Preview {
        std::string token;
        std::string owner;
        DisplayConfiguration requested;
        DisplayConfiguration rollbackRequest;
        AppliedConfiguration rollbackApplied;
        int64_t deadlineMs = 0;
    };

    static AppliedConfiguration makeApplied(const HardwareSnapshot &hardware,
                                            const DisplayConfigurationValidator::Result &validated,
                                            uint64_t revision);
    static DisplayConfiguration makeRequest(const HardwareSnapshot &hardware,
                                            const AppliedConfiguration &applied);
    std::string nextToken(const char *prefix) const;
    void finish(const std::string &operationToken, ConfigurationResult result);
    void startRevert(DisplayErrorCode reason, std::string message,
                     std::string failedOperation = {});

    DisplayManager *m_manager = nullptr;
    NativeDisplayDelegate *m_delegate = nullptr;
    DisplayConfigurator m_configurator;
    DisplayPreferences m_preferences;
    base::Timer m_previewTimer;
    base::Subscription m_hardwareInvalidated;
    std::optional<Preview> m_preview;
    bool m_operationActive = false;
    bool m_topologyPending = false;
    uint64_t m_hardwareEpoch = 0;
};

} // namespace Lingmo::Display

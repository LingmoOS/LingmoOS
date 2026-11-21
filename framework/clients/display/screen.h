// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/display/display_client.h"
#include "lingmo/display/output_model.h"

namespace Lingmo::Display {

// The displays as Settings shows them: the compositor's state, edits that
// apply as soon as they are made, and the confirmation a mode change waits for.
class Screen
{
public:
    // Tests name a display service of their own.
    explicit Screen(std::string serviceName = "org.lingmo.DisplayConfig");
    ~Screen();

    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    OutputModel& outputModel() { return m_outputModel; }
    const OutputModel& outputModel() const { return m_outputModel; }

    bool loaded() const { return m_client.loaded(); }
    bool available() const { return m_client.available(); }
    bool busy() const { return m_busy; }
    bool dirty() const { return m_outputModel.dirty(); }
    bool canApply() const;
    const std::string& error() const { return m_error; }

    bool pendingConfirmation() const { return !m_client.state().previewToken.empty(); }
    // Whole seconds left before the compositor reverts a previewed mode.
    int confirmationSeconds() const;

    void apply();
    void discard();
    void keep();
    void revert();

    // Anything above changed; while a confirmation is pending, also every
    // quarter second for its countdown.
    base::Signal<> stateChanged;

private:
    void updateState();

    DisplayClient m_client;
    OutputModel m_outputModel;
    base::Timer m_countdown;
    std::string m_instanceId;
    uint64_t m_topologyGeneration = 0;
    std::string m_error;
    bool m_busy = false;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Display

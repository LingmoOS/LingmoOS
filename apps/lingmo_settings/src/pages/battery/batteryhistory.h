// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "lingmo/upower/device.h"

namespace settings {

// A battery's charge or power draw over a span of time, from UPower's
// history, ready to plot: ordered by time, one sample per second at most,
// starting with the value in force at the span's start and running on to
// the present. With --mock-battery it holds sample days instead.
class BatteryHistory
{
public:
    enum class Type {
        Rate,
        Charge,
    };

    struct Query {
        // The UPower device object path; nothing loads while it is empty.
        std::string device;
        // Seconds back from now.
        uint32_t duration = 120;
        Type type = Type::Charge;

        bool operator==(const Query&) const = default;
    };

    BatteryHistory();
    ~BatteryHistory();

    BatteryHistory(const BatteryHistory&) = delete;
    BatteryHistory& operator=(const BatteryHistory&) = delete;

    // Loads unless `query` is already the one loaded.
    void load(const Query& query);
    // Loads the current query again, for samples recorded since.
    void refresh();
    const Query& query() const { return m_query; }

    // Empty until the first answer arrives.
    const std::vector<Lingmo::upower::HistorySample>& samples() const { return m_samples; }
    Lingmo::base::Signal<> samplesChanged;

    // The samples to plot from UPower's answers: the span's samples, newest
    // first, and the whole history when it came, over `duration` seconds up
    // to `now`.
    static std::vector<Lingmo::upower::HistorySample> plotted(
            const std::vector<Lingmo::upower::HistorySample>& window,
            const std::optional<std::vector<Lingmo::upower::HistorySample>>& all, uint32_t now,
            uint32_t duration);

private:
    struct Answers {
        std::optional<std::vector<Lingmo::upower::HistorySample>> window;
        std::optional<std::vector<Lingmo::upower::HistorySample>> all;
        int outstanding = 2;
    };

    void request();
    void answered(const std::shared_ptr<Answers>& answers);
    std::vector<Lingmo::upower::HistorySample> mockSamples() const;

    Query m_query;
    bool m_loaded = false;
    std::unique_ptr<Lingmo::upower::Device> m_device;
    std::vector<Lingmo::upower::HistorySample> m_samples;
    std::shared_ptr<Answers> m_answers;
    Lingmo::base::dbus::PendingCall m_windowCall;
    Lingmo::base::dbus::PendingCall m_allCall;
};

} // namespace settings

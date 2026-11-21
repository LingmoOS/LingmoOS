// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/battery/batteryhistory.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <limits>

#include "window/options.h"

using Lingmo::upower::Device;
using Lingmo::upower::HistorySample;

namespace settings {

BatteryHistory::BatteryHistory() = default;

BatteryHistory::~BatteryHistory() = default;

void BatteryHistory::load(const Query& query)
{
    if (m_loaded && query == m_query)
        return;

    if (query.device != m_query.device)
        m_device.reset();
    m_query = query;
    m_loaded = true;
    request();
}

void BatteryHistory::refresh()
{
    if (m_loaded)
        request();
}

void BatteryHistory::request()
{
    if (m_query.device.empty())
        return;

    if (options().mockBattery) {
        m_samples = mockSamples();
        samplesChanged.emit();
        return;
    }

    if (!m_device)
        m_device = std::make_unique<Device>(m_query.device);

    const Device::HistoryType type
            = m_query.type == Type::Rate ? Device::HistoryType::Rate : Device::HistoryType::Charge;
    // A query that changes meanwhile leaves these answers unused.
    auto answers = std::make_shared<Answers>();
    m_answers = answers;

    m_windowCall = m_device->history(type, m_query.duration, 100,
                                     [this, answers](const std::optional<std::vector<HistorySample>>& samples) {
                                         answers->window = samples;
                                         answered(answers);
                                     });

    // UPower returns only the samples inside the timespan and records one only
    // when the value changes, so the value at the window's start is the newest
    // sample before it, found in the whole history.
    m_allCall = m_device->history(type, std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(),
                                  [this, answers](const std::optional<std::vector<HistorySample>>& samples) {
                                      answers->all = samples;
                                      answered(answers);
                                  });
}

void BatteryHistory::answered(const std::shared_ptr<Answers>& answers)
{
    if (--answers->outstanding > 0 || answers != m_answers)
        return;

    m_answers.reset();
    if (!answers->window) {
        fprintf(stderr, "Failed to get device history from UPower\n");
        m_samples.clear();
        return;
    }

    m_samples = plotted(*answers->window, answers->all, uint32_t(std::time(nullptr)), m_query.duration);
    samplesChanged.emit();
}

std::vector<HistorySample> BatteryHistory::plotted(const std::vector<HistorySample>& window,
                                                   const std::optional<std::vector<HistorySample>>& all,
                                                   uint32_t now, uint32_t duration)
{
    std::vector<HistorySample> values;
    const uint32_t start = now > duration ? now - duration : 0;

    // The sample in force at the start. Restart markers are skipped here as
    // they are inside the window, or the start goes blank once one follows it.
    if (all) {
        HistorySample before;

        for (auto it = all->crbegin(); it != all->crend(); ++it) {
            if (it->time > start)
                break;

            if (it->value <= 0 || it->time < before.time)
                continue;

            before = *it;
        }

        if (before.value > 0) {
            before.time = start;
            values.push_back(before);
        }
    }

    // UPower returns the newest sample first, in the order it recorded them.
    // A sample older than the one recorded before it was stamped while the
    // clock was wrong (before NTP set it after boot), so it is dropped.
    for (auto it = window.crbegin(); it != window.crend(); ++it) {
        // A zero value marks a UPower restart, not a reading.
        if (it->value <= 0 || it->time > now)
            continue;

        if (!values.empty() && it->time < values.back().time)
            continue;

        // The curve needs distinct times; of two readings in a second, keep the later.
        if (!values.empty() && it->time == values.back().time) {
            values.back() = *it;
            continue;
        }

        values.push_back(*it);
    }

    // UPower records a sample only when the value changes, so the last one
    // still holds and the plot runs on to the present.
    if (!values.empty() && values.back().time < now) {
        HistorySample current = values.back();
        current.time = now;
        values.push_back(current);
    }

    return values;
}

std::vector<HistorySample> BatteryHistory::mockSamples() const
{
    std::vector<HistorySample> samples;
    const uint32_t now = uint32_t(std::time(nullptr));
    // Each day drains on battery, charges, is held at 90% by the charge
    // limit and is unplugged again, ending where the next day starts.
    static const std::vector<double> chargeValues{72, 66, 59, 52, 45, 38, 32, 44, 57,
                                                  69, 80, 88, 90, 90, 86, 81, 77, 72};
    static const std::vector<double> rateValues{7.2, 6.8, 7.5, 6.9, 7.1, 6.4, 6.0, 11.8, 12.4,
                                                11.1, 9.6, 7.2, 4.1, 3.9, 6.8, 7.4, 6.9, 6.6};
    static const std::vector<uint32_t> states{2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 5, 2, 2, 2, 2};
    // Days drain a little deeper or shallower than one another.
    static const std::vector<double> depths{0, -6, 4, -3, 6, -4, 2};
    const std::vector<double>& values = m_query.type == Type::Rate ? rateValues : chargeValues;
    const uint32_t duration = m_query.duration;
    const uint32_t days = std::max(1u, duration / 86400);
    const uint32_t interval = duration / days / uint32_t(values.size());

    for (uint32_t day = 0; day < days; ++day) {
        for (size_t i = 0; i < values.size(); ++i) {
            HistorySample sample;
            sample.time = now - duration + (day * uint32_t(values.size()) + uint32_t(i)) * interval;
            sample.value = values[i];
            sample.state = states[i];

            if (m_query.type == Type::Charge && i >= 3 && i <= 8)
                sample.value += depths[(days - 1 - day) % depths.size()];

            samples.push_back(sample);
        }
    }

    HistorySample current = samples.back();
    current.time = now;
    samples.push_back(current);
    return samples;
}

} // namespace settings

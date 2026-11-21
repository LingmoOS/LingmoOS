// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/finger_track.h"

#include <algorithm>
#include <iterator>

namespace Lingmo::Scene::Motion {

void FingerTrack::add(double timeMs, double position)
{
    if (!m_samples.empty())
        timeMs = std::max(timeMs, m_samples.back().timeMs);
    m_samples.push_back({timeMs, position});
    while (m_samples.size() > 2 && timeMs - m_samples.front().timeMs > kVelocityHorizonMs * 2)
        m_samples.pop_front();
}

void FingerTrack::shift(double by)
{
    for (Sample &sample : m_samples)
        sample.position += by;
}

double FingerTrack::position(double timeMs) const
{
    const auto after = std::upper_bound(m_samples.begin(), m_samples.end(), timeMs,
                                        [](double time, const Sample &sample) {
                                            return time < sample.timeMs;
                                        });
    if (after == m_samples.begin())
        return after->position;
    const auto lerp = [](const Sample &a, const Sample &b, double time) {
        const double dt = b.timeMs - a.timeMs;
        return dt > 0 ? a.position + (b.position - a.position) * (time - a.timeMs) / dt
                      : b.position;
    };
    if (after != m_samples.end())
        return lerp(*std::prev(after), *after, timeMs);

    // Ahead of the newest event: along the last two, not far.
    const Sample &newest = m_samples.back();
    if (m_samples.size() < 2)
        return newest.position;
    const Sample &previous = m_samples[m_samples.size() - 2];
    const double dt = newest.timeMs - previous.timeMs;
    if (dt < kResampleMinDeltaMs)
        return newest.position;
    return lerp(previous, newest,
                std::min(timeMs, newest.timeMs + std::min(kResampleMaxPredictionMs, dt / 2)));
}

double FingerTrack::releaseVelocity(double releaseTimeMs) const
{
    if (m_samples.size() < 2 || releaseTimeMs - m_samples.back().timeMs > kAssumeReleaseStoppedMs)
        return 0;
    // A first-degree least-squares fit over the recent samples, back to where
    // the fingers last paused.
    const double newest = m_samples.back().timeMs;
    auto first = std::prev(m_samples.end());
    while (first != m_samples.begin()) {
        const auto previous = std::prev(first);
        if (newest - previous->timeMs > kVelocityHorizonMs
            || first->timeMs - previous->timeMs > kAssumeMoveStoppedMs)
            break;
        first = previous;
    }
    const double count = double(std::distance(first, m_samples.end()));
    if (count < 2)
        return 0;
    double meanTime = 0;
    double meanPosition = 0;
    for (auto it = first; it != m_samples.end(); ++it) {
        meanTime += it->timeMs / count;
        meanPosition += it->position / count;
    }
    double covariance = 0;
    double variance = 0;
    for (auto it = first; it != m_samples.end(); ++it) {
        covariance += (it->timeMs - meanTime) * (it->position - meanPosition);
        variance += (it->timeMs - meanTime) * (it->timeMs - meanTime);
    }
    return variance > 1e-6 ? covariance / variance * 1000 : 0;
}

std::optional<double> FingerTrack::recentVelocity() const
{
    if (m_samples.size() < 2)
        return std::nullopt;
    const Sample &newest = m_samples.back();
    const Sample &recent = m_samples[m_samples.size() - std::min<size_t>(m_samples.size(), 3)];
    if (newest.timeMs - recent.timeMs < kResampleMinDeltaMs)
        return std::nullopt;
    return (newest.position - recent.position) / (newest.timeMs - recent.timeMs) * 1000;
}

double FingerTrack::predictionEnd() const
{
    const Sample &newest = m_samples.back();
    if (m_samples.size() < 2)
        return newest.timeMs;
    const double dt = newest.timeMs - m_samples[m_samples.size() - 2].timeMs;
    return dt < kResampleMinDeltaMs ? newest.timeMs
                                    : newest.timeMs + std::min(kResampleMaxPredictionMs, dt / 2);
}

} // namespace Lingmo::Scene::Motion

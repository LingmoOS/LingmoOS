// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/dock_bounce.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Lingmo::Shell {

DockBounce::DockBounce(ui::Window *window)
    : FrameClient(window)
{
}

DockBounce::~DockBounce() = default;

void DockBounce::start(const void *icon, double size, int hops)
{
    if (!icon)
        return;
    const double duration = std::clamp(hops, 1, kBounces) * kPeriodMs;
    for (Hop &hop : m_hops) {
        if (hop.icon != icon)
            continue;
        // Starting again while it hops: from the top, so it reads as a click.
        hop.startMs = -1;
        hop.size = size;
        hop.durationMs = duration;
        if (!isTicking())
            startTicking();
        return;
    }
    m_hops.push_back(Hop {icon, -1, size, duration});
    startTicking();
}

void DockBounce::stop(const void *icon)
{
    const size_t removed = std::erase_if(m_hops, [icon](const Hop &hop) { return hop.icon == icon; });
    if (removed == 0)
        return;
    if (m_hops.empty()) {
        stopTicking();
        m_lastFrameNs = 0;
    }
    if (onChanged)
        onChanged();
}

void DockBounce::clear()
{
    if (m_hops.empty())
        return;
    m_hops.clear();
    stopTicking();
    m_lastFrameNs = 0;
    if (onChanged)
        onChanged();
}

double DockBounce::lift(const void *icon) const
{
    for (const Hop &hop : m_hops) {
        if (hop.icon != icon)
            continue;
        if (hop.startMs < 0)
            return 0;
        const double elapsed = m_nowMs - hop.startMs;
        if (elapsed <= 0 || elapsed >= hop.durationMs)
            return 0;
        // Hops: |sin| of one period, each lower than the one before.
        return hop.size * kAmplitude * std::abs(std::sin(std::numbers::pi * elapsed / kPeriodMs))
                * std::exp(-kDecay * elapsed / hop.durationMs);
    }
    return 0;
}

void DockBounce::tick(uint64_t frameTimeNs)
{
    m_lastFrameNs = frameTimeNs;
    m_nowMs = double(frameTimeNs) / 1e6;
    for (Hop &hop : m_hops) {
        // The first frame after the click starts the hop from the top.
        if (hop.startMs < 0)
            hop.startMs = m_nowMs;
    }
    std::erase_if(m_hops, [this](const Hop &hop) { return m_nowMs - hop.startMs >= hop.durationMs; });
    if (m_hops.empty()) {
        stopTicking();
        m_lastFrameNs = 0;
        if (onChanged)
            onChanged();
        return;
    }
    // Every frame lifts the hop somewhere else.
    if (onChanged)
        onChanged();
}

} // namespace Lingmo::Shell

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/animation.h"

#include <algorithm>
#include <cmath>


namespace Lingmo::ui {

Animation::Animation(Window* window)
    : FrameClient(window)
{
}

void Animation::start()
{
    m_running = true;
    m_paused = false;
    m_startNs = 0;
    m_elapsedNs = 0;
    startTicking();
}

void Animation::stop()
{
    m_running = false;
    m_paused = false;
    stopTicking();
}

void Animation::pause()
{
    if (!m_running || m_paused)
        return;
    m_paused = true;
    stopTicking();
}

void Animation::resume()
{
    if (!m_running || !m_paused)
        return;
    m_paused = false;
    // Re-anchored on the next tick, keeping the progress made so far.
    m_startNs = 0;
    startTicking();
}

void Animation::tick(uint64_t frameTimeNs)
{
    if (!m_startNs)
        m_startNs = frameTimeNs - m_elapsedNs;
    m_elapsedNs = frameTimeNs - m_startNs;

    const double duration = std::max<double>(1, double(m_duration.count()) * 1e6);
    const double cycles = double(m_elapsedNs) / duration;
    if (m_loops != Infinite && cycles >= m_loops) {
        const bool backwards = m_alternating && m_loops % 2 == 0;
        stop();
        if (onUpdate)
            onUpdate(backwards ? 0.0 : 1.0);
        if (onFinished)
            onFinished();
        return;
    }

    const double cycle = std::floor(cycles);
    double t = cycles - cycle;
    if (m_alternating && int64_t(cycle) % 2 == 1)
        t = 1.0 - t;
    if (onUpdate)
        onUpdate(ease(t));
}

double Animation::ease(double t) const
{
    switch (m_easing) {
    case Easing::Linear:
        return t;
    case Easing::InOutCubic:
        return t < 0.5 ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
    case Easing::OutCubic:
        return 1 - std::pow(1 - t, 3);
    case Easing::OutSine:
        return std::sin(t * M_PI / 2);
    case Easing::InOutQuad:
        return t < 0.5 ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
    case Easing::OutQuad:
        return 1 - (1 - t) * (1 - t);
    case Easing::InQuad:
        return t * t;
    }
    return t;
}

} // namespace Lingmo::ui

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>

#include "ui/frame_client.h"

namespace Lingmo::ui {

// Runs on its window's frames: every rendered frame ticks it once.
class Animation : public FrameClient
{
public:
    enum class Easing {
        Linear,
        InOutCubic,
        OutCubic,
        OutSine,
        InOutQuad,
        OutQuad,
        InQuad,
    };

    static constexpr int Infinite = -1;

    explicit Animation(Window* window);

    void setDuration(std::chrono::milliseconds duration) { m_duration = duration; }
    void setEasing(Easing easing) { m_easing = easing; }
    void setLoops(int loops) { m_loops = loops; }
    // Every other loop runs backwards.
    void setAlternating(bool alternating) { m_alternating = alternating; }

    void start();
    void stop();
    void pause();
    void resume();
    bool isRunning() const { return m_running; }

    // The eased progress, from 0 to 1.
    std::function<void(double value)> onUpdate;
    std::function<void()> onFinished;

protected:
    void tick(uint64_t frameTimeNs) override;

private:
    double ease(double t) const;

    std::chrono::milliseconds m_duration{250};
    Easing m_easing = Easing::InOutCubic;
    int m_loops = 1;
    bool m_alternating = false;
    bool m_running = false;
    bool m_paused = false;
    uint64_t m_startNs = 0;
    uint64_t m_elapsedNs = 0;
};

} // namespace Lingmo::ui

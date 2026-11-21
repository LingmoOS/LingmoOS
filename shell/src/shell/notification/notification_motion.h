// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <cstdint>
#include <functional>

#include "ui/frame_client.h"

namespace Lingmo::Shell {

// The notifications' motion: 380 ms on cubic-bezier(0.25, 1, 0.5, 1).
inline constexpr std::chrono::milliseconds kMotionDuration {380};
double motionEasing(double progress);
// Out cubic: 1 - (1 - t)^3.
double outCubic(double progress);

// One value animated on its window's frames: a new target starts from what
// is drawn now; without animation
// it is there at once.
class Tween : public ui::FrameClient
{
public:
    using Easing = double (*)(double);

    explicit Tween(ui::Window *window, std::chrono::milliseconds duration = kMotionDuration,
                   Easing easing = motionEasing);

    double value() const { return m_value; }
    double target() const { return m_to; }
    void setDuration(std::chrono::milliseconds duration) { m_duration = duration; }

    void setTarget(double target, bool animate);
    // Stops where it is going.
    void snap(double value);

    // The value changed, on a frame or at once.
    std::function<void()> onChanged;
    std::function<void()> onFinished;

protected:
    void tick(uint64_t frameTimeNs) override;

private:
    std::chrono::milliseconds m_duration;
    Easing m_easing;
    double m_value = 0;
    double m_from = 0;
    double m_to = 0;
    uint64_t m_startNs = 0;
};

} // namespace Lingmo::Shell


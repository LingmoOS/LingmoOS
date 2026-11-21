// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <vector>

#include "ui/frame_client.h"

namespace Lingmo::Shell {

// The dock's launch bounce, as macOS has it: an application that is starting
// hops up and down a few times, each hop lower than the one before, and stops
// when its window appears or when the hops are used up. Several may bounce at
// once, one per icon.
class DockBounce : public ui::FrameClient
{
public:
    // The highest hop, as a fraction of the icon size.
    static constexpr double kAmplitude = 0.18;
    // One hop up and down.
    static constexpr double kPeriodMs = 300;
    static constexpr int kBounces = 3;
    static constexpr double kDurationMs = kPeriodMs * kBounces;
    // An attention request hops twice: enough to be noticed, short enough to
    // leave the icon still if the user is looking elsewhere.
    static constexpr int kAttentionHops = 2;
    // How much the hops die away over the three of them.
    static constexpr double kDecay = 1.1;

    explicit DockBounce(ui::Window *window);
    ~DockBounce() override;

    // Starts, or restarts, the icon's bounce; `size` is the icon's size, and
    // `hops` of kBounces how long it lasts.
    void start(const void *icon, double size, int hops = kBounces);
    void stop(const void *icon);
    void clear();
    bool isRunning() const { return isTicking(); }
    // How far the icon is lifted now, along the dock's normal; 0 at rest.
    double lift(const void *icon) const;

    std::function<void()> onChanged;

protected:
    void tick(uint64_t frameTimeNs) override;

private:
    struct Hop
    {
        const void *icon = nullptr;
        // When the hop started on the frame clock; the first frame after
        // start() gives it its time.
        double startMs = -1;
        double size = 0;
        double durationMs = kDurationMs;
    };

    std::vector<Hop> m_hops;
    double m_nowMs = 0;
    uint64_t m_lastFrameNs = 0;
};

} // namespace Lingmo::Shell

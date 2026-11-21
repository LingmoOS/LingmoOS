// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/system/night_light/night_light_controller_impl.cc
//               (ColorTemperatureAnimation, MatrixFromTemperature,
//                BlueColorScaleFromTemperature, GreenColorScaleFromTemperature)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the compositor only animates the strength it is given
// and tints every output through KMS; the schedule and the preferences belong
// to lingmo-services, which calls DisplayConfig1.SetNightLight.

#pragma once

#include <chrono>
#include <functional>

#include "base/timer.h"
#include "render/color_transform.h"

namespace Lingmo::Display {

// The night light's strength, from 0, which leaves the screen untouched, to 1,
// the warmest, and the tint each step of its animation gives the outputs.
class NightLight
{
public:
    using Clock = std::function<std::chrono::steady_clock::time_point()>;
    using Apply = std::function<void(const render::ColorTransform &)>;

    explicit NightLight(Apply apply, Clock clock = std::chrono::steady_clock::now);
    NightLight(const NightLight &) = delete;
    NightLight &operator=(const NightLight &) = delete;

    // Moves linearly from where the animation is now. Ash takes a second for a
    // change the user makes and a minute for a scheduled one.
    void animateTo(double strength, std::chrono::milliseconds duration);

    double strength() const { return m_current; }

    double target() const { return m_target; }

    bool isAnimating() const { return m_timer.isActive(); }

    // Red stays; blue is toned down, and green half as much so the screen does
    // not turn green.
    static render::ColorTransform transformFor(double strength);

    // One animation step, as the timer runs it, for tests that drive the clock.
    void stepForTesting() { step(); }

private:
    void step();

    Apply m_apply;
    Clock m_clock;
    base::Timer m_timer;
    double m_start = 0;
    double m_current = 0;
    double m_target = 0;
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::milliseconds m_duration {0};
};

} // namespace Lingmo::Display

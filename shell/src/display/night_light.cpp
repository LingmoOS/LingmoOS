// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/system/night_light/night_light_controller_impl.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "display/night_light.h"

#include <algorithm>
#include <utility>

#include "core/main_loop.h"

namespace Lingmo::Display {

namespace {

// kNightLightAnimationFrameRate: a slow tint needs no more steps than this.
constexpr int kAnimationFrameRate = 15;

} // namespace

NightLight::NightLight(Apply apply, Clock clock)
    : m_apply(std::move(apply))
    , m_clock(std::move(clock))
    , m_timer(core::mainLoop())
{
    m_timer.setInterval(std::chrono::milliseconds(1000 / kAnimationFrameRate));
    m_timer.onTimeout = [this] { step(); };
}

void NightLight::animateTo(double strength, std::chrono::milliseconds duration)
{
    m_start = m_current;
    m_target = std::clamp(strength, 0.0, 1.0);
    m_startTime = m_clock();
    m_duration = duration;
    if (duration.count() > 0 && m_start != m_target) {
        m_timer.start();
        return;
    }
    m_timer.stop();
    m_current = m_target;
    m_apply(transformFor(m_current));
}

render::ColorTransform NightLight::transformFor(double strength)
{
    // GreenColorScaleFromTemperature() and BlueColorScaleFromTemperature().
    return {1.0f, float(1.0 - 0.5 * strength), float(1.0 - strength)};
}

void NightLight::step()
{
    const double elapsed =
            std::chrono::duration<double, std::milli>(m_clock() - m_startTime).count();
    const double state = m_duration.count() > 0
            ? std::clamp(elapsed / double(m_duration.count()), 0.0, 1.0)
            : 1.0;
    m_current = m_start + (m_target - m_start) * state;
    if (state >= 1.0) {
        m_current = m_target;
        m_timer.stop();
    }
    m_apply(transformFor(m_current));
}

} // namespace Lingmo::Display

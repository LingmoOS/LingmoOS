// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <cmath>
#include <functional>
#include <memory>

#include "ui/animation.h"
#include "ui/frame_client.h"

namespace Lingmo::Shell {

// The easing curves the launcher's animations and transitions use.
namespace easing {

inline double linear(double t)
{
    return t;
}

inline double outCubic(double t)
{
    const double u = t - 1;
    return u * u * u + 1;
}

inline double inCubic(double t)
{
    return t * t * t;
}

// Out back with an overshoot of 1.70158.
inline double outBack(double t)
{
    constexpr double s = 1.70158;
    const double u = t - 1;
    return u * u * ((s + 1) * u + s) + 1;
}

inline double inOutQuad(double t)
{
    return t < 0.5 ? 2 * t * t : -2 * t * t + 4 * t - 1;
}

} // namespace easing

// A value moving from one number to another on the window's frames, along
// one of the curves above.
class LauncherTween
{
public:
    using Curve = double (*)(double);

    LauncherTween() = default;
    LauncherTween(const LauncherTween &) = delete;
    LauncherTween &operator=(const LauncherTween &) = delete;

    // From `from` to `to`; `update` hears each value, the last one `to`.
    void start(ui::Window *window, double from, double to, std::chrono::milliseconds duration, Curve curve,
               std::function<void(double)> update)
    {
        m_to = to;
        m_update = std::move(update);
        if (!window || duration.count() <= 0) {
            finish();
            return;
        }
        if (!m_animation || m_animation->window() != window)
            m_animation = std::make_unique<ui::Animation>(window);
        m_animation->stop();
        m_animation->setEasing(ui::Animation::Easing::Linear);
        m_animation->setDuration(duration);
        m_running = true;
        m_animation->onUpdate = [this, from, to, curve](double t) {
            m_value = from + (to - from) * curve(t);
            if (m_update)
                m_update(m_value);
        };
        m_animation->onFinished = [this] {
            m_running = false;
            m_value = m_to;
        };
        m_value = from;
        m_animation->start();
    }

    // Jumps to the end.
    void finish()
    {
        if (m_animation)
            m_animation->stop();
        m_running = false;
        m_value = m_to;
        if (m_update)
            m_update(m_to);
    }

    // Leaves the value where it is.
    void stop()
    {
        if (m_animation)
            m_animation->stop();
        m_running = false;
    }

    bool isRunning() const { return m_running; }
    double target() const { return m_to; }

private:
    std::unique_ptr<ui::Animation> m_animation;
    std::function<void(double)> m_update;
    double m_value = 0;
    double m_to = 0;
    bool m_running = false;
};

// A value pulled onto its target by a critically damped spring on the
// window's frames: it eases out of rest, and started again towards another
// target it keeps the speed it has, so a move that changes course bends
// instead of jerking.
class LauncherSpring
{
public:
    LauncherSpring() = default;
    LauncherSpring(const LauncherSpring &) = delete;
    LauncherSpring &operator=(const LauncherSpring &) = delete;

    // From `from` to `to`, keeping its speed if already running; `update`
    // hears each value, the last one `to`.
    void start(ui::Window *window, double from, double to, std::function<void(double)> update)
    {
        m_value = from;
        m_to = to;
        m_update = std::move(update);
        if (!window) {
            finish();
            return;
        }
        if (!m_driver || m_driver->window() != window) {
            m_driver = std::make_unique<Driver>(window, this);
            m_running = false;
            m_velocity = 0;
        }
        if (!m_running) {
            m_running = true;
            m_driver->start();
        }
    }

    // Jumps to the end.
    void finish()
    {
        stop();
        m_value = m_to;
        if (m_update)
            m_update(m_to);
    }

    // Leaves the value where it is.
    void stop()
    {
        if (m_driver)
            m_driver->stop();
        m_running = false;
        m_velocity = 0;
    }

    bool isRunning() const { return m_running; }
    double target() const { return m_to; }

private:
    // Radians per second: a 0.25 s response, nearly still after 0.2 s.
    static constexpr double kFrequency = 2 * 3.14159265358979 / 0.25;
    static constexpr double kRestDistance = 0.1;
    static constexpr double kRestSpeed = 5.0;

    class Driver final : public ui::FrameClient
    {
    public:
        Driver(ui::Window *window, LauncherSpring *spring)
            : FrameClient(window)
            , m_spring(spring)
        {
        }

        void start()
        {
            m_lastNs = 0;
            startTicking();
        }

        void stop() { stopTicking(); }

    protected:
        void tick(uint64_t frameTimeNs) override
        {
            // The first frame already shows one step of motion.
            const double seconds = m_lastNs ? double(frameTimeNs - m_lastNs) / 1e9 : 1.0 / 60;
            m_lastNs = frameTimeNs;
            m_spring->step(seconds);
        }

    private:
        LauncherSpring *m_spring;
        uint64_t m_lastNs = 0;
    };

    void step(double seconds)
    {
        const double offset = m_value - m_to;
        const double coefficient = m_velocity + kFrequency * offset;
        const double decay = std::exp(-kFrequency * seconds);
        const double next = (offset + coefficient * seconds) * decay;
        m_velocity = (m_velocity - kFrequency * coefficient * seconds) * decay;
        if (std::abs(next) < kRestDistance && std::abs(m_velocity) < kRestSpeed) {
            finish();
            return;
        }
        m_value = m_to + next;
        if (m_update)
            m_update(m_value);
    }

    std::unique_ptr<Driver> m_driver;
    std::function<void(double)> m_update;
    double m_value = 0;
    double m_velocity = 0;
    double m_to = 0;
    bool m_running = false;
};

} // namespace Lingmo::Shell


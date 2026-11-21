// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_SCENE_MOTION_H_
#define LINGMO_SCENE_MOTION_H_

namespace Lingmo::Scene::Motion {

// How a spring moves, in the response / damping-ratio form. `response` is the
// period of the undamped oscillation in seconds; a critically damped spring
// (dampingRatio 1) started from rest covers 99% of its distance in about that
// time and never overshoots. Below 1 it overshoots, above 1 it creeps.
struct SpringParameters {
    double response = 0.35;
    double dampingRatio = 1;
};

// --animation-duration-scale stretches every spring and tween, the way
// ui::ScopedAnimationDurationScaleMode stretches Chromium's animations, so
// motion can be judged in slow motion. 1 unless set to a positive number.
double durationScale();
void setDurationScale(double scale);

// gfx::CubicBezier::Solve for a timing curve from (0, 0) to (1, 1), sampled at
// `x`, which is clamped to [0, 1].
double cubicBezier(double x1, double y1, double x2, double y2, double x);

// A damped harmonic oscillator evaluated in closed form. Its position depends
// only on the time it is sampled at, never on how often or how regularly it
// was sampled before, so it can be stepped on output frame times directly.
// Times are milliseconds on the frame clock; velocities are units per second.
class Spring
{
public:
    Spring();
    explicit Spring(const SpringParameters &parameters);

    // Starts at `value`, moving at `velocity`, toward `target`, at `time`.
    void reset(double time, double value, double target, double velocity = 0);

    // Heads for `target` from wherever the spring is at `time`, keeping its
    // velocity, so the motion bends toward the new target instead of stopping.
    void retarget(double time, double target);

    double value(double time) const;
    double velocity(double time) const;

    double target() const { return m_target; }

private:
    // Displacement from the target and its rate, `elapsed` seconds after m_start.
    double displacement(double elapsed) const;
    double displacementVelocity(double elapsed) const;
    double elapsed(double time) const;
    void updateCoefficients();

    SpringParameters m_parameters;
    double m_omega = 0;
    double m_zeta = 1;
    double m_start = 0;
    double m_displacement = 0;
    double m_velocity = 0;
    double m_target = 0;
};

// A cubic Bézier timing curve from (0, 0) to (1, 1), as CSS and gfx::Tween
// write them.
struct TimingCurve {
    double x1;
    double y1;
    double x2;
    double y2;
};
// gfx::Tween::ACCEL_20_DECEL_100: off at once, a long soft landing.
inline constexpr TimingCurve kAccel20Decel100 {0.2, 0, 0, 1};
// Under way within a few frames and landing softly, below twice its average
// speed from rest or bent to a slower start, so it never lunges.
inline constexpr TimingCurve kQuickStart {0.2, 0, 0.4, 1};

// A timed move on a timing curve whose start is bent to leave at a given
// speed, so a release or a turn carries on without a kink. It never
// overshoots: a start away from the target is not kept, and one faster than
// the curve can begin at shortens the move instead.
class Tween
{
public:
    Tween() = default;
    // From `from` at `time` to `target` within `durationMs`, leaving at
    // `velocity` units per second.
    Tween(double time, double from, double target, double durationMs, double velocity = 0,
          const TimingCurve &curve = kAccel20Decel100);

    double value(double time) const;
    double velocity(double time) const;

    double target() const { return m_target; }
    bool isFinished(double time) const { return time >= m_start + m_duration; }

private:
    double m_start = 0;
    double m_from = 0;
    double m_target = 0;
    double m_duration = 0;
    // The curve's control points, the first one's y bent to the start speed.
    double m_x1 = kAccel20Decel100.x1;
    double m_y1 = kAccel20Decel100.y1;
    double m_x2 = kAccel20Decel100.x2;
    double m_y2 = kAccel20Decel100.y2;
};

// A critically damped spring from `from` to `target` at `time`, keeping
// `velocity` up to the speed the spring would bring it in at: faster, it
// would shoot past the target and show what lies beyond, so even a hard
// flick leaves the rest of the way to a visible slide.
Spring settlingSpring(double time, double from, double target, double velocity, double response);

} // namespace Lingmo::Scene::Motion

#endif // LINGMO_SCENE_MOTION_H_

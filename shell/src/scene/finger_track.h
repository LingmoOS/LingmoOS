// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_SCENE_FINGER_TRACK_H_
#define LINGMO_SCENE_FINGER_TRACK_H_

#include <deque>
#include <optional>

namespace Lingmo::Scene::Motion {

// Where the fingers of a touchpad swipe put something, event by event: shown
// at each frame's time the way ui::LinearResampling does, and fitted for the
// speed they let go at the way ui::VelocityTracker does. Positions are in
// whatever unit the caller moves; velocities are that unit per second.
class FingerTrack
{
public:
    // ui::LinearResampling: the fingers are shown where they were this long
    // before the frame, and never guessed further ahead of the newest event
    // than the prediction limit or half the last event interval.
    static constexpr double kResampleLatencyMs = 5;
    static constexpr double kResampleMaxPredictionMs = 8;
    static constexpr double kResampleMinDeltaMs = 2;
    // ui::VelocityTracker: the fit's horizon, and the gaps after which the
    // fingers count as having stopped.
    static constexpr double kVelocityHorizonMs = 100;
    static constexpr double kAssumeMoveStoppedMs = 40;
    static constexpr double kAssumeReleaseStoppedMs = 80;

    struct Sample {
        double timeMs = 0;
        double position = 0;
    };

    // Events arriving out of order are taken at the newest time seen.
    void add(double timeMs, double position);
    void clear() { m_samples.clear(); }
    bool empty() const { return m_samples.empty(); }

    // Moves every position by `by`, as when what the positions count from moves.
    void shift(double by);

    // Where the fingers were at `timeMs`: between two events along both, and
    // past the newest along the last two, not far. Needs an event.
    double position(double timeMs) const;

    // The least-squares speed over the last events, back to where the fingers
    // last paused; 0 when they had stopped before `releaseTimeMs`.
    double releaseVelocity(double releaseTimeMs) const;

    // The speed over the last few events alone, which a stop shows in at once
    // where the fit lags; empty when they came too close together to tell.
    std::optional<double> recentVelocity() const;

    // The time position() stops following the events and holds.
    double predictionEnd() const;

    // Whether a frame at `frameTime` still shows the fingers moving on.
    bool isCatchingUp(double frameTime) const
    {
        return !m_samples.empty()
                && frameTime - kResampleLatencyMs
                        < m_samples.back().timeMs + kResampleMaxPredictionMs;
    }

private:
    std::deque<Sample> m_samples;
};

} // namespace Lingmo::Scene::Motion

#endif // LINGMO_SCENE_FINGER_TRACK_H_

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <vector>

#include "ui/event.h"
#include "ui/frame_client.h"
#include "ui/geometry.h"

namespace Lingmo::ui {

// Touchpad and wheel scrolling of one view. It follows lingmo-framework's
// Scroller, so every view in the session scrolls alike: finger
// samples are resampled to each frame of the window, and the coast and the
// rubber band run on the same frame clock. A paged view moves along one axis
// and settles on a viewport-sized page.
class Scroller : public FrameClient
{
public:
    struct Axis
    {
        double minimum = 0;
        double maximum = 0;
        double viewport = 0;
        bool enabled = false;
    };

    // The view being scrolled.
    class Target
    {
    public:
        virtual ~Target() = default;
        virtual Axis horizontalAxis() const = 0;
        virtual Axis verticalAxis() const = 0;
        virtual PointF position() const = 0;
        // A position past the bounds shows the view stretched over its edge.
        virtual void applyPosition(PointF position) = 0;
        // From the first movement until the view comes to rest.
        virtual void movingChanged(bool) {}
    };

    // A frame shows the fingers this far in the past, so most frames interpolate.
    static constexpr double resampleLatency = 5;

    Scroller(Window* window, Target* target);
    ~Scroller() override;

    void setElastic(bool elastic) { m_elastic = elastic; }
    void setPaged(bool paged) { m_paged = paged; }

    // Whether the event moved the view, or belongs to a gesture that did.
    bool handleScroll(const ScrollEvent& event);
    // A paged view lands on the page it was heading for.
    void cancel(bool restoreBounds = true);
    // Ends a coast where it is shown; an overscrolled view still settles.
    void stopCoast();
    // Something else moved the view.
    void viewMoved();
    bool isActive() const { return m_gesture || m_coasting; }
    bool isScrolling() const { return m_scrolling; }

    // Milliseconds; tests replace it.
    static double clock();
    static void setClock(double (*clock)());

protected:
    void tick(uint64_t frameTimeNs) override;
    void interrupt(Interruption reason) override;

private:
    enum Axes {
        NoAxes = 0,
        Horizontal = 1,
        Vertical = 2,
        BothAxes = 3,
    };

    struct Sample
    {
        double time;
        PointF position;
    };

    // Shows the frame for frameTime, in clock() milliseconds.
    void advance(double frameTime);
    void setPosition(PointF position);
    void scheduleFrame();
    double eventTime(const ScrollEvent& event) const;
    void reset(bool restoreBounds);
    void settle();
    void begin();
    bool handleDelta(PointF delta, double time, bool wheel, uint32_t modifiers);
    void addSample(double time);
    void release(double time);
    void startCoast(PointF position, double time);
    PointF contentPosition(PointF dragPosition) const;
    PointF resampled(double time, bool extrapolate, bool* extrapolated = nullptr) const;
    PointF flickVelocity() const;
    double dragPosition(double position, const Axis& axis) const;
    double unboundedPosition(double position, const Axis& axis) const;
    void advanceAxis(double& position, double& velocity, const Axis& axis, double seconds);
    double pageTarget(double position, double velocity, const Axis& axis) const;
    double nearestPage(double position, const Axis& axis) const;
    void updateScrolling();

    Target* m_target;
    std::vector<Sample> m_samples;
    PointF m_dragPosition;
    PointF m_shownDragPosition;
    PointF m_coastPosition;
    PointF m_velocity;
    PointF m_snapTarget;
    PointF m_snapDuration;
    PointF m_snapVelocity;
    PointF m_accumulatedDelta;
    double m_shownTime;
    double m_coastTime = 0;
    int m_axes = BothAxes;
    int m_dragAxes = NoAxes;
    int m_wheelStepTimer = 0;
    bool m_inputGesture = false;
    bool m_axisChosen = false;
    bool m_gesture = false;
    bool m_coasting = false;
    bool m_extrapolated = false;
    bool m_consumed = false;
    bool m_cancelled = false;
    bool m_writing = false;
    bool m_elastic = true;
    bool m_paged = false;
    bool m_scrolling = false;
    bool m_frameRequested = false;
};

} // namespace Lingmo::ui

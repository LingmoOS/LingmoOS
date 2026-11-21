// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/gestures/wm_gesture_handler.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: upstream switches desks with four fingers and keeps
// three for its overview; here three fingers sideways switch desks and three
// fingers up and down bring the overview in and take it away, both following
// the fingers. libinput swipes stand in for ui::ScrollEvent. The desk content
// follows the fingers whatever the scrolling direction is set to, and by the
// distance they travel on the pad rather than by accelerated pointer motion,
// which would run ahead of a quick swipe and lag a slow one. A swipe that
// starts while something is being dragged, a window or data, switches
// nothing: the drag would carry across desks mid-flight.

#ifndef LINGMO_WM_GESTURES_WM_GESTURE_HANDLER_H_
#define LINGMO_WM_GESTURES_WM_GESTURE_HANDLER_H_

#include <functional>

#include "input/gesture_filter.h"

namespace Lingmo::WM {

class DesksController;
class OverviewController;

class WmGestureHandler : public input::GestureFilter
{
public:
    static constexpr uint32_t kDeskSwitchFingers = 3;

    // How far, in millimetres, the fingers move before a continuous animation
    // starts, to leave out accidental swipes.
    static constexpr double kContinuousGestureMoveThresholdMm = 2;

    // The threshold's distance is made up over this many times as much travel
    // after the start, rather than landing in one frame.
    static constexpr double kCatchUpTravelRatio = 2;

    // Finger travel that moves the desk content by one desk.
    static constexpr double kSwipeLengthPerDeskMm = 80;

    // Finger travel that brings the overview all the way in or out.
    static constexpr double kOverviewSwipeLengthMm = 50;

    // libinput's unaccelerated touchpad motion is a thousandth of an inch
    // scaled by the adaptive filter's constant 0.9 * 0.2968 (filter-touchpad.c,
    // touchpad_constant_filter); touchpads keep that profile here.
    static constexpr double kMmPerUnacceleratedUnit = 25.4 / 1000 / (0.9 * 0.2968);

    // `dragging` tells whether a drag is running; empty means never. Without
    // `overview`, three fingers up and down do nothing.
    explicit WmGestureHandler(DesksController *desks, std::function<bool()> dragging = {},
                              OverviewController *overview = nullptr);

    // Called as a swipe this handler takes starts, before it moves anything:
    // the first frame it brings would meet an idle GPU otherwise.
    void setWarmUp(std::function<void()> warmUp) { m_warmUp = std::move(warmUp); }

    struct PinchActions {
        std::function<bool(bool inward)> begin;
        std::function<void(double progress)> update;
        std::function<void(bool cancelled)> end;
    };
    void setPinchActions(PinchActions actions) { m_pinchActions = std::move(actions); }
    bool claimGesture(input::Timestamp timestamp, input::GestureType type,
                      uint32_t fingers) override;
    void onGestureUpdate(input::Timestamp timestamp, const input::GestureUpdate &update) override;
    void onGestureEnd(input::Timestamp timestamp, bool cancelled) override;

private:
    // `delta` along the started axis, with its share of what is owed.
    double catchUp(double delta);

    struct ScrollData {
        // In millimetres.
        double scrollX = 0;
        double scrollY = 0;
        bool horizontalContinuousGestureStarted = false;
        bool verticalContinuousGestureStarted = false;
        // Threshold distance the started animation has yet to make up, and
        // the travel along it since the start.
        double owed = 0;
        double owedTravel = 0;
        // The time of the gesture's last event so far, its begin included.
        double timeMs = 0;
        // The overview could not start, or a desk switch could not: the rest
        // of the gesture does nothing.
        bool ignored = false;
    };

    DesksController *m_desks = nullptr;
    std::function<bool()> m_dragging;
    OverviewController *m_overview = nullptr;
    std::function<void()> m_warmUp;
    ScrollData m_scroll;
    PinchActions m_pinchActions;
    bool m_pinch = false;
    int m_pinchDirection = 0;
    bool m_pinchStarted = false;
    bool m_pinchIgnored = false;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_GESTURES_WM_GESTURE_HANDLER_H_

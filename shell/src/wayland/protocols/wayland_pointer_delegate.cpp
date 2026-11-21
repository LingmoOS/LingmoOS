// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_pointer_delegate.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Adaptation: the aura scroll unit and its conversion factor are gone; libinput
// already reports wheel clicks and finger scrolls in the units Wayland wants.

#include "wayland/protocols/wayland_pointer_delegate.h"

#include <cmath>
#include <linux/input-event-codes.h>
#include <wayland-server-protocol.h>

#include "core/logging.h"
#include "input/seat.h"
#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

// Weston, the reference compositor, treats one wheel click as 10 axis units,
// and every toolkit calibrates against that. Offsets arrive here already in
// those units; these constants only recover the click fractions for
// axis_value120 and axis_discrete.
constexpr double kWaylandScrollUnit = 10.0;
constexpr int kValue120PerClick = 120;

} // namespace

WaylandPointerDelegate::WaylandPointerDelegate(wl_resource *pointerResource,
                                               SerialTracker *serialTracker,
                                               clipboard::SelectionController *selection,
                                               input::Seat *seat)
    : m_seat(seat)
    , m_pointerResource(pointerResource)
    , m_serialTracker(serialTracker)
    , m_selection(selection ? selection->weakPtr() : nullptr)
{
}

WaylandPointerDelegate::~WaylandPointerDelegate() = default;

wl_client *WaylandPointerDelegate::client() const
{
    return wl_resource_get_client(m_pointerResource);
}

void WaylandPointerDelegate::onPointerDestroying(input::Pointer *)
{
    delete this;
}

bool WaylandPointerDelegate::canAcceptPointerEventsForSurface(compositor::Surface *surface) const
{
    wl_resource *surfaceResource = getSurfaceResource(surface);
    return surfaceResource && wl_resource_get_client(surfaceResource) == client();
}

void WaylandPointerDelegate::onPointerEnter(compositor::Surface *surface,
                                            const base::PointF &location,
                                            input::ButtonFlags)
{
    wl_resource *surfaceResource = getSurfaceResource(surface);
    if (!surfaceResource)
        return;
    m_pendingValue120 = {};

    // Exo notes the open question of whether buttons already down should be
    // reported before the enter. The protocol has no way to say so, and
    // toolkits do not expect it, so the flags are only used to seed our own
    // state.
    const auto serial = m_serialTracker->nextSerial(SerialTracker::POINTER_ENTER);
    LM_DEBUG(lingmoCursorTransitions) << "enter pointer" << wl_resource_get_id(m_pointerResource)
                                  << "client" << client() << "serial" << serial << "surface"
                                  << surface;
    m_enterSerial = serial;
    m_serialTracker->recordActivationSerial(serial, m_pointerResource);
    const base::PointF local = logicalToClient(location, clientScale(client()));
    wl_pointer_send_enter(m_pointerResource, serial, surfaceResource,
                          wl_fixed_from_double(local.x()), wl_fixed_from_double(local.y()));
}

void WaylandPointerDelegate::onPointerLeave(compositor::Surface *surface)
{
    m_enterSerial.reset();
    wl_resource *surfaceResource = getSurfaceResource(surface);
    if (!surfaceResource)
        return;

    wl_pointer_send_leave(m_pointerResource,
                          m_serialTracker->nextSerial(SerialTracker::POINTER_LEAVE),
                          surfaceResource);
}

void WaylandPointerDelegate::onPointerMotion(input::Timestamp timestamp,
                                             const base::PointF &location)
{
    const base::PointF local = logicalToClient(location, clientScale(client()));
    wl_pointer_send_motion(m_pointerResource, timePointToMilliseconds(timestamp),
                           wl_fixed_from_double(local.x()), wl_fixed_from_double(local.y()));
}

void WaylandPointerDelegate::onPointerButton(input::Timestamp timestamp,
                                             input::ButtonFlags changedButtonFlags, bool pressed)
{
    constexpr struct {
        input::ButtonFlag flag;
        uint32_t value;
        SerialTracker::EventType down;
        SerialTracker::EventType up;
    } buttons[] = {
            {input::LeftButton, BTN_LEFT, SerialTracker::POINTER_LEFT_BUTTON_DOWN,
             SerialTracker::POINTER_LEFT_BUTTON_UP},
            {input::RightButton, BTN_RIGHT, SerialTracker::POINTER_RIGHT_BUTTON_DOWN,
             SerialTracker::POINTER_RIGHT_BUTTON_UP},
            {input::MiddleButton, BTN_MIDDLE, SerialTracker::POINTER_MIDDLE_BUTTON_DOWN,
             SerialTracker::POINTER_MIDDLE_BUTTON_UP},
            {input::ForwardButton, BTN_EXTRA, SerialTracker::POINTER_FORWARD_BUTTON_DOWN,
             SerialTracker::POINTER_FORWARD_BUTTON_UP},
            {input::BackButton, BTN_SIDE, SerialTracker::POINTER_BACK_BUTTON_DOWN,
             SerialTracker::POINTER_BACK_BUTTON_UP},
    };

    for (const auto &button : buttons) {
        if (!(changedButtonFlags & button.flag))
            continue;
        const SerialTracker::EventType eventType = pressed ? button.down : button.up;
        const auto serial = m_serialTracker->nextSerial(eventType);
        if (m_selection)
            m_selection->recordSerial(client(), serial);
        if (pressed)
            m_serialTracker->recordActivationSerial(serial, m_pointerResource);
        if (pressed && m_seat)
            m_seat->recordDragSerial(serial, button.flag);
        wl_pointer_send_button(
                m_pointerResource, serial, timePointToMilliseconds(timestamp), button.value,
                pressed ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED);
    }
}

void WaylandPointerDelegate::onPointerScroll(input::Timestamp timestamp,
                                             const base::PointF &offset,
                                             bool discrete)
{
    if (wl_resource_get_version(m_pointerResource) >= WL_POINTER_AXIS_SOURCE_SINCE_VERSION) {
        wl_pointer_send_axis_source(m_pointerResource,
                                    discrete ? WL_POINTER_AXIS_SOURCE_WHEEL
                                             : WL_POINTER_AXIS_SOURCE_FINGER);
    }

    // Exo negates here because aura's scroll axes point the other way. libinput
    // already reports positive as down/right, which is Wayland's sense.
    const double xValue = offset.x();
    const double yValue = offset.y();

    if (discrete) {
        sendWheelSteps(WL_POINTER_AXIS_HORIZONTAL_SCROLL, xValue);
        sendWheelSteps(WL_POINTER_AXIS_VERTICAL_SCROLL, yValue);
    }

    wl_pointer_send_axis(m_pointerResource, timePointToMilliseconds(timestamp),
                         WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(xValue));
    wl_pointer_send_axis(m_pointerResource, timePointToMilliseconds(timestamp),
                         WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(yValue));
}

// Both events must precede the axis event they describe.
void WaylandPointerDelegate::sendWheelSteps(uint32_t axis, double value)
{
    const int value120 =
            static_cast<int>(std::lround(value / kWaylandScrollUnit * kValue120PerClick));
    if (value120 == 0)
        return;
    const int version = wl_resource_get_version(m_pointerResource);
    if (version >= WL_POINTER_AXIS_VALUE120_SINCE_VERSION) {
        wl_pointer_send_axis_value120(m_pointerResource, axis, value120);
        return;
    }
    if (version < WL_POINTER_AXIS_DISCRETE_SINCE_VERSION)
        return;

    // Exo rounds every wheel event up to a whole click, which holds for aura's
    // wheel events. A high-resolution wheel reports fractions of one, so they
    // add up until a click is complete, as in KWin and wlroots; turning back
    // starts over.
    int &pending = m_pendingValue120[axis];
    if ((pending > 0 && value120 < 0) || (pending < 0 && value120 > 0))
        pending = 0;
    pending += value120;
    const int clicks = pending / kValue120PerClick;
    if (clicks == 0)
        return;
    pending -= clicks * kValue120PerClick;
    wl_pointer_send_axis_discrete(m_pointerResource, axis, clicks);
}

void WaylandPointerDelegate::onFingerScrollStop(input::Timestamp timestamp)
{
    if (wl_resource_get_version(m_pointerResource) < WL_POINTER_AXIS_STOP_SINCE_VERSION)
        return;

    wl_pointer_send_axis_source(m_pointerResource, WL_POINTER_AXIS_SOURCE_FINGER);
    wl_pointer_send_axis_stop(m_pointerResource, timePointToMilliseconds(timestamp),
                              WL_POINTER_AXIS_HORIZONTAL_SCROLL);
    wl_pointer_send_axis_stop(m_pointerResource, timePointToMilliseconds(timestamp),
                              WL_POINTER_AXIS_VERTICAL_SCROLL);
}

void WaylandPointerDelegate::onPointerFrame()
{
    if (wl_resource_get_version(m_pointerResource) >= WL_POINTER_FRAME_SINCE_VERSION)
        wl_pointer_send_frame(m_pointerResource);
    wl_client_flush(client());
}

} // namespace Lingmo::Wayland

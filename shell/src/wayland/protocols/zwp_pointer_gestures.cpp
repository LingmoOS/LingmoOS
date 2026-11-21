// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_pointer_gestures.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: Exo implements pinch alone, at version 1, with a
// scale and no translation or rotation. libinput has all of them, so swipe,
// pinch and hold are served at version 3, with serials from the seat's
// tracker and motion in the client's units.

#include "wayland/protocols/zwp_pointer_gestures.h"

#include <algorithm>
#include <pointer-gestures-unstable-v1-server-protocol.h>

#include "input/pointer.h"
#include "input/pointer_gesture_delegate.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

constexpr uint32_t kPointerGesturesVersion = 3;

class WaylandPointerGestureDelegate : public input::PointerGestureDelegate
{
public:
    WaylandPointerGestureDelegate(wl_resource *resource, input::GestureType type,
                                  input::Pointer *pointer, SerialTracker *serialTracker)
        : m_resource(resource)
        , m_type(type)
        , m_pointer(pointer)
        , m_serialTracker(serialTracker)
    {
        if (m_pointer)
            m_pointer->addGestureDelegate(this);
    }

    WaylandPointerGestureDelegate(const WaylandPointerGestureDelegate &) = delete;
    WaylandPointerGestureDelegate &operator=(const WaylandPointerGestureDelegate &) = delete;

    ~WaylandPointerGestureDelegate() override
    {
        if (m_pointer)
            m_pointer->removeGestureDelegate(this);
    }

    void onPointerDestroying(input::Pointer *) override { m_pointer = nullptr; }

    input::GestureType gestureType() const override { return m_type; }

    void onPointerGestureBegin(input::Timestamp timestamp, compositor::Surface *surface,
                               uint32_t fingers) override
    {
        wl_resource *surfaceResource = getSurfaceResource(surface);
        if (!surfaceResource)
            return;
        const uint32_t serial = m_serialTracker->nextSerial(SerialTracker::OTHER_EVENT);
        const uint32_t time = timePointToMilliseconds(timestamp);
        switch (m_type) {
        case input::GestureType::Swipe:
            zwp_pointer_gesture_swipe_v1_send_begin(m_resource, serial, time, surfaceResource,
                                                    fingers);
            break;
        case input::GestureType::Pinch:
            zwp_pointer_gesture_pinch_v1_send_begin(m_resource, serial, time, surfaceResource,
                                                    fingers);
            break;
        case input::GestureType::Hold:
            zwp_pointer_gesture_hold_v1_send_begin(m_resource, serial, time, surfaceResource,
                                                   fingers);
            break;
        }
    }

    void onPointerGestureUpdate(input::Timestamp timestamp,
                                const input::GestureUpdate &update) override
    {
        const base::PointF delta =
                logicalToClient(update.delta, clientScale(wl_resource_get_client(m_resource)));
        const uint32_t time = timePointToMilliseconds(timestamp);
        if (m_type == input::GestureType::Swipe) {
            zwp_pointer_gesture_swipe_v1_send_update(m_resource, time,
                                                     wl_fixed_from_double(delta.x()),
                                                     wl_fixed_from_double(delta.y()));
        } else if (m_type == input::GestureType::Pinch) {
            zwp_pointer_gesture_pinch_v1_send_update(
                    m_resource, time, wl_fixed_from_double(delta.x()),
                    wl_fixed_from_double(delta.y()), wl_fixed_from_double(update.scale),
                    wl_fixed_from_double(update.rotation));
        }
    }

    void onPointerGestureEnd(input::Timestamp timestamp, bool cancelled) override
    {
        const uint32_t serial = m_serialTracker->nextSerial(SerialTracker::OTHER_EVENT);
        const uint32_t time = timePointToMilliseconds(timestamp);
        switch (m_type) {
        case input::GestureType::Swipe:
            zwp_pointer_gesture_swipe_v1_send_end(m_resource, serial, time, cancelled);
            break;
        case input::GestureType::Pinch:
            zwp_pointer_gesture_pinch_v1_send_end(m_resource, serial, time, cancelled);
            break;
        case input::GestureType::Hold:
            zwp_pointer_gesture_hold_v1_send_end(m_resource, serial, time, cancelled);
            break;
        }
    }

private:
    wl_resource *const m_resource;
    const input::GestureType m_type;
    input::Pointer *m_pointer;
    SerialTracker *const m_serialTracker;
};

void gesture_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct zwp_pointer_gesture_swipe_v1_interface swipe_implementation = {gesture_destroy};
const struct zwp_pointer_gesture_pinch_v1_interface pinch_implementation = {gesture_destroy};
const struct zwp_pointer_gesture_hold_v1_interface hold_implementation = {gesture_destroy};

void createGesture(wl_client *client, wl_resource *resource, uint32_t id,
                   wl_resource *pointerResource, input::GestureType type)
{
    const wl_interface *interface = &zwp_pointer_gesture_swipe_v1_interface;
    const void *implementation = &swipe_implementation;
    if (type == input::GestureType::Pinch) {
        interface = &zwp_pointer_gesture_pinch_v1_interface;
        implementation = &pinch_implementation;
    } else if (type == input::GestureType::Hold) {
        interface = &zwp_pointer_gesture_hold_v1_interface;
        implementation = &hold_implementation;
    }
    wl_resource *gestureResource =
            wl_resource_create(client, interface, wl_resource_get_version(resource), id);
    if (!gestureResource) {
        wl_client_post_no_memory(client);
        return;
    }
    setImplementation(gestureResource, implementation,
                      std::make_unique<WaylandPointerGestureDelegate>(
                              gestureResource, type, pointerFromResource(pointerResource),
                              getUserDataAs<WaylandSeat>(resource)->serialTracker));
}

void pointer_gestures_get_swipe_gesture(wl_client *client, wl_resource *resource, uint32_t id,
                                        wl_resource *pointerResource)
{
    createGesture(client, resource, id, pointerResource, input::GestureType::Swipe);
}

void pointer_gestures_get_pinch_gesture(wl_client *client, wl_resource *resource, uint32_t id,
                                        wl_resource *pointerResource)
{
    createGesture(client, resource, id, pointerResource, input::GestureType::Pinch);
}

void pointer_gestures_release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void pointer_gestures_get_hold_gesture(wl_client *client, wl_resource *resource, uint32_t id,
                                       wl_resource *pointerResource)
{
    createGesture(client, resource, id, pointerResource, input::GestureType::Hold);
}

const struct zwp_pointer_gestures_v1_interface pointer_gestures_implementation = {
        pointer_gestures_get_swipe_gesture, pointer_gestures_get_pinch_gesture,
        pointer_gestures_release, pointer_gestures_get_hold_gesture};

void bind_pointer_gestures(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource = wl_resource_create(client, &zwp_pointer_gestures_v1_interface,
                                               std::min(version, kPointerGesturesVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &pointer_gestures_implementation, data, nullptr);
}

} // namespace

wl_global *createPointerGesturesGlobal(wl_display *display, WaylandSeat *seat)
{
    return wl_global_create(display, &zwp_pointer_gestures_v1_interface, kPointerGesturesVersion,
                            seat, bind_pointer_gestures);
}

} // namespace Lingmo::Wayland

// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_seat.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wayland/protocols/wl_seat.h"

#include <algorithm>
#include <memory>
#include <wayland-server-protocol.h>

#include "compositor/surface.h"
#include "core/logging.h"
#include "input/keyboard.h"
#include "input/pointer.h"
#include "input/seat.h"
#include "input/touch.h"
#include "wayland/protocols/wayland_keyboard_delegate.h"
#include "wayland/protocols/wayland_pointer_delegate.h"
#include "wayland/protocols/wayland_touch_delegate.h"
#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

////////////////////////////////////////////////////////////////////////////////
// wl_pointer_interface:

void pointer_set_cursor(wl_client *client, wl_resource *resource, uint32_t serial,
                        wl_resource *surfaceResource, int32_t hotspotX, int32_t hotspotY)
{
    LM_DEBUG(lingmoCursorTransitions) << "request pointer" << wl_resource_get_id(resource) << "client"
                                  << wl_resource_get_client(resource) << "enter-serial" << serial;
    auto *pointer = getUserDataAs<input::Pointer>(resource);
    auto *delegate = static_cast<WaylandPointerDelegate *>(pointer->delegate());
    if (!delegate->acceptsCursorSerial(serial))
        return;
    pointer->setCursor(surfaceFromResource(surfaceResource),
                       clientToLogical(base::Point(hotspotX, hotspotY), clientScale(client)));
}

void pointer_release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct wl_pointer_interface pointer_implementation = {pointer_set_cursor, pointer_release};

////////////////////////////////////////////////////////////////////////////////
// wl_keyboard_interface:

void keyboard_release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct wl_keyboard_interface keyboard_implementation = {keyboard_release};

////////////////////////////////////////////////////////////////////////////////
// wl_touch_interface:

void touch_release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct wl_touch_interface touch_implementation = {touch_release};

////////////////////////////////////////////////////////////////////////////////
// wl_seat_interface:

void seat_get_pointer(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *data = getUserDataAs<WaylandSeat>(resource);

    wl_resource *pointerResource = wl_resource_create(client, &wl_pointer_interface,
                                                      wl_resource_get_version(resource), id);
    if (!pointerResource) {
        wl_client_post_no_memory(client);
        return;
    }

    // The delegate deletes itself from Pointer::onPointerDestroying, as it does
    // upstream, so the Pointer owned by the resource is the only owner here.
    setImplementation(pointerResource, &pointer_implementation,
                      std::make_unique<input::Pointer>(
                              new WaylandPointerDelegate(pointerResource, data->serialTracker,
                                                         data->seat->selection(), data->seat),
                              data->seat));
}

void seat_get_keyboard(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *data = getUserDataAs<WaylandSeat>(resource);

    wl_resource *keyboardResource = wl_resource_create(client, &wl_keyboard_interface,
                                                       wl_resource_get_version(resource), id);
    if (!keyboardResource) {
        wl_client_post_no_memory(client);
        return;
    }

    setImplementation(
            keyboardResource, &keyboard_implementation,
            std::make_unique<input::Keyboard>(
                    std::make_unique<WaylandKeyboardDelegate>(keyboardResource, data->serialTracker,
                                                              data->seat->selection()),
                    data->seat));
}

void seat_get_touch(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *data = getUserDataAs<WaylandSeat>(resource);

    wl_resource *touchResource =
            wl_resource_create(client, &wl_touch_interface, wl_resource_get_version(resource), id);
    if (!touchResource) {
        wl_client_post_no_memory(client);
        return;
    }

    // As for the pointer, the delegate deletes itself when the Touch goes.
    setImplementation(touchResource, &touch_implementation,
                      std::make_unique<input::Touch>(
                              new WaylandTouchDelegate(touchResource, data->serialTracker,
                                                       data->seat->selection()),
                              data->seat));
}

void seat_release(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct wl_seat_interface seat_implementation = {seat_get_pointer, seat_get_keyboard,
                                                      seat_get_touch, seat_release};

void bind_seat(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource =
            wl_resource_create(client, &wl_seat_interface, std::min(version, kWlSeatVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &seat_implementation, data, nullptr);

    if (wl_resource_get_version(resource) >= WL_SEAT_NAME_SINCE_VERSION)
        wl_seat_send_name(resource, "default");
    // Touch is always offered, as upstream does: a client without a
    // touchscreen simply never gets a touch event.
    wl_seat_send_capabilities(resource, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD
                                                | WL_SEAT_CAPABILITY_TOUCH);
}

} // namespace

WaylandSeat::WaylandSeat(input::Seat *seat, SerialTracker *serialTracker)
    : seat(seat)
    , serialTracker(serialTracker)
{
    seat->addObserver(this);
    onSurfaceFocused(seat->focusedSurface(), nullptr);
}

WaylandSeat::~WaylandSeat()
{
    seat->removeObserver(this);
}

void WaylandSeat::onSurfaceFocused(compositor::Surface *gained, compositor::Surface *)
{
    auto *resource = getSurfaceResource(gained);
    seat->primarySelection()->setRecipient(resource ? wl_resource_get_client(resource) : nullptr);
    seat->selection()->setRecipient(resource ? wl_resource_get_client(resource) : nullptr);
}

input::Pointer *pointerFromResource(wl_resource *resource)
{
    return resource
                    && wl_resource_instance_of(resource, &wl_pointer_interface,
                                               &pointer_implementation)
            ? getUserDataAs<input::Pointer>(resource)
            : nullptr;
}

WaylandSeat *waylandSeatFromResource(wl_resource *resource)
{
    return resource && wl_resource_instance_of(resource, &wl_seat_interface, &seat_implementation)
            ? getUserDataAs<WaylandSeat>(resource)
            : nullptr;
}

wl_global *createSeatGlobal(wl_display *display, WaylandSeat *data)
{
    return wl_global_create(display, &wl_seat_interface, kWlSeatVersion, data, bind_seat);
}

} // namespace Lingmo::Wayland

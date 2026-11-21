// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/cursor_shape.h"

#include <algorithm>
#include <wayland-server-core.h>

#include "cursor-shape-v1-server-protocol.h"
#include "input/pointer.h"
#include "wayland/protocols/wayland_pointer_delegate.h"
#include "wayland/protocols/wl_seat.h"

namespace Lingmo::Wayland {

namespace {

// The extension may outlive its wl_pointer. Keep a resource destroy listener,
// not a raw model pointer, and make the extension inert before model teardown.
struct ShapeDevice {
    wl_listener pointerDestroyed;
    wl_resource* pointerResource = nullptr;

    explicit ShapeDevice(wl_resource* pointer)
        : pointerResource(pointer)
    {
        wl_list_init(&pointerDestroyed.link);
        pointerDestroyed.notify = [](wl_listener* listener, void*) {
            auto* self = reinterpret_cast<ShapeDevice*>(listener);
            self->pointerResource = nullptr;
            wl_list_remove(&listener->link);
            wl_list_init(&listener->link);
        };
        if (pointer)
            wl_resource_add_destroy_listener(pointer, &pointerDestroyed);
    }

    ~ShapeDevice() { wl_list_remove(&pointerDestroyed.link); }
};

void destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void setShape(wl_client*, wl_resource* resource, uint32_t serial, uint32_t shape)
{
    // Advertise version 1: the complete CSS set, without tablet support or
    // the additional version 2 shapes. Validate even on an inert device.
    if (shape < WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT ||
        shape > WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_OUT) {
        wl_resource_post_error(resource, WP_CURSOR_SHAPE_DEVICE_V1_ERROR_INVALID_SHAPE,
                               "Invalid cursor shape %u", shape);
        return;
    }
    auto* device = static_cast<ShapeDevice*>(wl_resource_get_user_data(resource));
    auto* pointer = pointerFromResource(device->pointerResource);
    if (!pointer)
        return;
    auto* delegate = static_cast<WaylandPointerDelegate*>(pointer->delegate());
    if (!delegate->acceptsCursorSerial(serial))
        return;
    // CursorShape deliberately follows the standard's stable shape values.
    pointer->setCursorShape(static_cast<input::CursorShape>(shape));
}

const struct wp_cursor_shape_device_v1_interface deviceImplementation = {destroy, setShape};

void createDevice(wl_client* client, wl_resource* manager, uint32_t id, wl_resource* pointer)
{
    auto* resource = wl_resource_create(client, &wp_cursor_shape_device_v1_interface,
                                        wl_resource_get_version(manager), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(
        resource, &deviceImplementation, new ShapeDevice(pointer), [](wl_resource* resource) {
            delete static_cast<ShapeDevice*>(wl_resource_get_user_data(resource));
        });
}

void getTabletTool(wl_client* client, wl_resource* manager, uint32_t id, wl_resource*)
{
    // No tablet manager is advertised. Keep the constructor inert if tablet
    // support is added later, until its cursor policy is implemented.
    createDevice(client, manager, id, nullptr);
}

const struct wp_cursor_shape_manager_v1_interface managerImplementation = {destroy, createDevice,
                                                                           getTabletTool};

void bindManager(wl_client* client, void*, uint32_t version, uint32_t id)
{
    auto* resource = wl_resource_create(client, &wp_cursor_shape_manager_v1_interface,
                                        std::min(version, 1u), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, nullptr, nullptr);
}

} // namespace

wl_global* createCursorShapeGlobal(wl_display* display)
{
    return wl_global_create(display, &wp_cursor_shape_manager_v1_interface, 1, nullptr,
                            bindManager);
}

} // namespace Lingmo::Wayland

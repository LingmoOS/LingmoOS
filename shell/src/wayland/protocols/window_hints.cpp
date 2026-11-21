// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/window_hints.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <lingmo-window-hints-v1-server-protocol.h>

#include "base/signal.h"
#include "base/strings.h"
#include "compositor/surface.h"
#include "wayland/protocols/surface_blur.h"
#include "wayland/server_util.h"
#include "wm/window_manager.h"

namespace Lingmo::Wayland {
namespace {

// Every bound manager, for the events about a client's surfaces.
std::vector<wl_resource *> &managers()
{
    static std::vector<wl_resource *> resources;
    return resources;
}

struct Manager {
    wm::WindowManager *windows = nullptr;
    // What work_area last said of each output, by name: the area and
    // whether the output is the primary one.
    std::map<std::string, std::pair<base::Rect, bool>> sentWorkAreas;
};

// Follows the work areas for the managers of one global, until the display
// goes.
struct WorkAreaWatch {
    struct DisplayListener {
        wl_listener listener;
        WorkAreaWatch *watch;
    };

    DisplayListener displayDestroyed {};
    base::Subscription outputsChanged;
    base::Subscription zonesChanged;
};

void sendWorkAreas(wl_resource *resource)
{
    auto *manager = static_cast<Manager *>(wl_resource_get_user_data(resource));
    if (!manager->windows)
        return;
    for (const wm::WindowManager::OutputArea &area : manager->windows->outputAreas()) {
        if (area.name.empty() || area.workArea.isEmpty())
            continue;
        const std::pair<base::Rect, bool> entry {area.workArea, area.primary};
        auto [sent, inserted] = manager->sentWorkAreas.try_emplace(area.name, entry);
        if (!inserted && sent->second == entry)
            continue;
        sent->second = entry;
        lingmo_window_hints_v1_send_work_area(resource, area.name.c_str(), area.workArea.x(),
                                                area.workArea.y(), area.workArea.width(),
                                                area.workArea.height(), area.primary ? 1 : 0);
    }
}

void sendWorkAreas(const wm::WindowManager *windows)
{
    for (wl_resource *resource : managers()) {
        if (static_cast<Manager *>(wl_resource_get_user_data(resource))->windows == windows)
            sendWorkAreas(resource);
    }
}

void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void setSkipTaskbar(wl_client *, wl_resource *resource, wl_resource *surface, uint32_t skip)
{
    if (skip > 1) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "skip_taskbar must be zero or one");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setSkipTaskbar(skip != 0);
}

void setAlwaysOnTop(wl_client *, wl_resource *resource, wl_resource *surface, uint32_t onTop)
{
    if (onTop > 1) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "always_on_top must be zero or one");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setAlwaysOnTop(onTop != 0);
}

void setResizeAnchor(wl_client *, wl_resource *resource, wl_resource *surface, uint32_t anchor)
{
    if (anchor > LINGMO_WINDOW_HINTS_V1_RESIZE_ANCHOR_CENTER) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "unknown resize anchor");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setResizeAnchor(
            anchor == LINGMO_WINDOW_HINTS_V1_RESIZE_ANCHOR_CENTER
                    ? compositor::ResizeAnchor::Center
                    : compositor::ResizeAnchor::TopLeft);
}

void setServerResize(wl_client *, wl_resource *resource, wl_resource *surface, uint32_t enabled)
{
    if (enabled > 1) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "server_resize must be zero or one");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setServerResize(enabled != 0);
}

void setRestoreId(wl_client *, wl_resource *, wl_resource *surface, const char *id)
{
    getUserDataAs<compositor::Surface>(surface)->setRestoreId(base::strings::sanitizedUtf8(id));
}

void setRestoreSize(wl_client *, wl_resource *resource, wl_resource *surface, int32_t width,
                    int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "restore size must not be negative");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setRestoreSize(base::Size(width, height));
}

void setPlacementAnchor(wl_client *, wl_resource *resource, wl_resource *surface, int32_t x,
                        int32_t y, int32_t width, int32_t height, uint32_t anchor, uint32_t gravity)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "placement anchor size must not be negative");
        return;
    }
    if (anchor > LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_BOTTOM_RIGHT
        || gravity > LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_BOTTOM_RIGHT) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "unknown placement edge");
        return;
    }
    static_assert(int(compositor::PlacementEdge::Top) == LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_TOP);
    static_assert(int(compositor::PlacementEdge::BottomRight)
                  == LINGMO_WINDOW_HINTS_V1_PLACEMENT_EDGE_BOTTOM_RIGHT);
    getUserDataAs<compositor::Surface>(surface)->setPlacementAnchor(compositor::PlacementAnchor {
            base::Rect(x, y, width, height), compositor::PlacementEdge(anchor),
            compositor::PlacementEdge(gravity)});
}

void unsetPlacementAnchor(wl_client *, wl_resource *, wl_resource *surface)
{
    getUserDataAs<compositor::Surface>(surface)->setPlacementAnchor(std::nullopt);
}

void setBodyShape(wl_client *, wl_resource *resource, wl_resource *surface, wl_array *array)
{
    ui::Path shape;
    if (!pathFromArray(array, &shape)) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "malformed body shape");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setBodyShape(shape);
}

void unsetBodyShape(wl_client *, wl_resource *, wl_resource *surface)
{
    getUserDataAs<compositor::Surface>(surface)->setBodyShape({});
}

void setShadow(wl_client *, wl_resource *resource, wl_resource *surface, uint32_t shadow)
{
    if (shadow > 1) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "shadow must be zero or one");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setCastsShadow(shadow != 0);
}

void setOpacity(wl_client *, wl_resource *resource, wl_resource *surface, wl_fixed_t fixed)
{
    const double opacity = wl_fixed_to_double(fixed);
    if (opacity < 0 || opacity > 1) {
        wl_resource_post_error(resource, LINGMO_WINDOW_HINTS_V1_ERROR_INVALID_VALUE,
                               "opacity must be between zero and one");
        return;
    }
    getUserDataAs<compositor::Surface>(surface)->setOpacity(float(opacity));
}

void hideApplication(wl_client *, wl_resource *resource, wl_resource *surface)
{
    auto *manager = static_cast<Manager *>(wl_resource_get_user_data(resource));
    if (!manager->windows)
        return;
    wm::Window *window = manager->windows->windowForSurface(getUserDataAs<compositor::Surface>(surface));
    if (!window)
        return;
    // The shell's own Hide, so the dock and the menu bar keep the application
    // where it was rather than losing it.
    manager->windows->hideApplication(window);
}

const struct lingmo_window_hints_v1_interface implementation = {
        destroy, setSkipTaskbar, setAlwaysOnTop, setResizeAnchor,
        setServerResize, setRestoreId, setRestoreSize, setPlacementAnchor,
        unsetPlacementAnchor, setBodyShape, unsetBodyShape, setShadow,
        setOpacity, hideApplication};

void unbind(wl_resource *resource)
{
    auto &resources = managers();
    resources.erase(std::remove(resources.begin(), resources.end(), resource), resources.end());
    delete static_cast<Manager *>(wl_resource_get_user_data(resource));
}

void bind(wl_client *client, void *data, uint32_t, uint32_t id)
{
    auto *resource = wl_resource_create(client, &lingmo_window_hints_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &implementation,
                                   new Manager {static_cast<wm::WindowManager *>(data), {}},
                                   unbind);
    managers().push_back(resource);
    sendWorkAreas(resource);
}

void displayDestroyed(wl_listener *listener, void *)
{
    auto *watch = reinterpret_cast<WorkAreaWatch::DisplayListener *>(listener)->watch;
    wl_list_remove(&listener->link);
    delete watch;
}

} // namespace

void sendInteractiveResize(wl_resource *surface, bool active)
{
    wl_client *client = wl_resource_get_client(surface);
    for (wl_resource *resource : managers()) {
        if (wl_resource_get_client(resource) == client)
            lingmo_window_hints_v1_send_interactive_resize(resource, surface, active ? 1 : 0);
    }
}

wl_global *createWindowHintsGlobal(wl_display *display, wm::WindowManager *windows)
{
    wl_global *global =
            wl_global_create(display, &lingmo_window_hints_v1_interface, 1, windows, bind);
    if (!global || !windows)
        return global;
    auto *watch = new WorkAreaWatch;
    watch->displayDestroyed.watch = watch;
    watch->displayDestroyed.listener.notify = displayDestroyed;
    wl_display_add_destroy_listener(display, &watch->displayDestroyed.listener);
    watch->outputsChanged = windows->outputsChanged.connect([windows] {
        sendWorkAreas(windows);
    });
    watch->zonesChanged = windows->exclusiveZonesChanged.connect([windows] {
        sendWorkAreas(windows);
    });
    return global;
}

} // namespace Lingmo::Wayland

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "View.h"

#include "Server.h"

#include <assert.h>

extern "C" {
#include <wlr/version.h>
}

#if WLR_VERSION_MINOR < 17
#define wlr_xdg_surface_try_from_wlr_surface(SURFACE) wlr_xdg_surface_from_wlr_surface(SURFACE)
#endif

WL_ADDONS_BASE_USE_NAMESPACE

View::View(Server *server, wlr_xdg_surface *xdg_surface, wl_list *list)
    : server_(server)
    , list_(list)
    , xdg_surface_(xdg_surface)
    , xdg_toplevel_(xdg_surface->toplevel)
    , scene_tree_(wlr_scene_xdg_surface_create(&server_->scene()->tree, xdg_surface_))
    , xdg_surface_map_(this)
    , xdg_surface_unmap_(this)
    , xdg_surface_destroy_(this)
    , xdg_toplevel_request_move_(this)
    , xdg_toplevel_request_resize_(this)
    , xdg_toplevel_request_maximize_(this)
    , xdg_toplevel_request_fullscreen_(this)
{
    scene_tree_->node.data = this;
    xdg_surface->data = scene_tree_;

    /* Listen to the various events it can emit */
#if WLR_VERSION_MINOR >= 17
    wl_signal_add(&xdg_surface->surface->events.map, xdg_surface_map_);
    wl_signal_add(&xdg_surface->surface->events.unmap, xdg_surface_unmap_);
#else
    wl_signal_add(&xdg_surface->events.map, xdg_surface_map_);
    wl_signal_add(&xdg_surface->events.unmap, xdg_surface_unmap_);
#endif

    wl_signal_add(&xdg_surface->events.destroy, xdg_surface_destroy_);

    /* cotd */
    wl_signal_add(&xdg_toplevel_->events.request_move, xdg_toplevel_request_move_);
    wl_signal_add(&xdg_toplevel_->events.request_resize, xdg_toplevel_request_resize_);
    wl_signal_add(&xdg_toplevel_->events.request_maximize, xdg_toplevel_request_maximize_);
    wl_signal_add(&xdg_toplevel_->events.request_fullscreen, xdg_toplevel_request_fullscreen_);

    auto *client = wl_resource_get_client(xdg_toplevel_->resource);
    wl_client_get_credentials(client, &pid_, nullptr, nullptr);
}

View::~View() { }

void View::xdgSurfaceMapNotify(void *data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */

    wl_list_insert(list_, &link_);

    focusView();
}

void View::xdgSurfaceUnmapNotify(void *data)
{
    /* Called when the surface is unmapped, and should no longer be shown. */

    /* Reset the cursor mode if the grabbed view was unmapped. */
    // if (view == view->server->grabbed_view) {
    //     reset_cursor_mode(view->server);
    // }

    wl_list_remove(&link_);
}

void View::xdgSurfaceDestroyNotify(void *data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    delete this;
}

void View::xdgToplevelRequestMoveNotify(void *data)
{
    /* This event is raised when a client would like to begin an interactive
     * move, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    // beginInteractive(TINYWL_CURSOR_MOVE, 0);
}

void View::xdgToplevelRequestResizeNotify(void *data)
{
    /* This event is raised when a client would like to begin an interactive
     * resize, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provided serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    auto *event = static_cast<wlr_xdg_toplevel_resize_event *>(data);
    // beginInteractive(TINYWL_CURSOR_RESIZE, event->edges);
}

void View::xdgToplevelRequestMaximizeNotify(void *data)
{
    /* This event is raised when a client would like to maximize itself,
     * typically because the user clicked on the maximize button on
     * client-side decorations. tinywl doesn't support maximization, but
     * to conform to xdg-shell protocol we still must send a configure.
     * wlr_xdg_surface_schedule_configure() is used to send an empty reply. */
    wlr_xdg_surface_schedule_configure(xdg_surface_);
}

void View::xdgToplevelRequestFullscreenNotify(void *data)
{
    /* Just as with request_maximize, we must send a configure here. */
    wlr_xdg_surface_schedule_configure(xdg_surface_);
}

void View::focusView()
{
    /* Note: this function only deals with keyboard focus. */
    struct wlr_surface *surface = xdg_surface_->surface;
    struct wlr_seat *seat = server_->seat();
    struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
    if (prev_surface == surface) {
        /* Don't re-focus an already focused surface. */
        return;
    }
    if (prev_surface) {
        /*
         * Deactivate the previously focused surface. This lets the client know
         * it no longer has focus and the client will repaint accordingly, e.g.
         * stop displaying a caret.
         */
        struct wlr_xdg_surface *previous =
            wlr_xdg_surface_try_from_wlr_surface(seat->keyboard_state.focused_surface);
        assert(previous->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
        wlr_xdg_toplevel_set_activated(previous->toplevel, false);
    }
    /* Move the view to the front */
    wlr_scene_node_raise_to_top(&scene_tree_->node);
    wl_list_remove(&link_);
    wl_list_insert(server_->views(), &link_);
    /* Activate the new surface */
    wlr_xdg_toplevel_set_activated(xdg_toplevel_, true);
    /*
     * Tell the seat to have the keyboard enter this surface. wlroots will keep
     * track of this and automatically send key events to the appropriate
     * clients without additional work on your part.
     */
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    if (keyboard != NULL) {
        wlr_seat_keyboard_notify_enter(seat,
                                       xdg_surface_->surface,
                                       keyboard->keycodes,
                                       keyboard->num_keycodes,
                                       &keyboard->modifiers);
    }
}

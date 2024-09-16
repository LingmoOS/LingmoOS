// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Server.h"

#include "InputMethodV2.h"
#include "Keyboard.h"
#include "Output.h"
#include "TextInputV3.h"
#include "View.h"
#include "X11ActiveWindowMonitor.h"

extern "C" {
#include <wlr/backend/wayland.h>
#include <wlr/backend/x11.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#define delete delete_
#include <wlr/types/wlr_input_method_v2.h>
#undef delete
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_text_input_v3.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/util/log.h>
#include <wlr/version.h>
}

#include <stdexcept>

#include <assert.h>

#if WLR_VERSION_MINOR < 17
#define wlr_xdg_surface_try_from_wlr_surface(SURFACE) wlr_xdg_surface_from_wlr_surface(SURFACE)
#define wlr_scene_surface_try_from_buffer(BUFFER) wlr_scene_surface_from_buffer(BUFFER)
#endif

struct unsafe_wlr_x11_backend
{
    struct wlr_backend backend;
    struct wl_display *wl_display;
    bool started;

    xcb_connection_t *xcb;
    xcb_screen_t *screen;
    xcb_depth_t *depth;
};

struct unsafe_wlr_x11_output
{
    struct wlr_output wlr_output;
    struct wlr_x11_backend *x11;
    struct wl_list link; // wlr_x11_backend::outputs

    xcb_window_t win;
};

Server::Server()
    : display_(wl_display_create())
    , backend_new_output_(this)
    , xdg_shell_new_surface_(this)
    , cursor_motion_(this)
    , cursor_motion_absolute_(this)
    , cursor_button_(this)
    , cursor_axis_(this)
    , cursor_frame_(this)
    , seat_request_cursor_(this)
    , seat_request_set_selection_(this)
    , backend_new_input_(this)
    , virtual_keyboard_manager_v1_new_virtual_keyboard_(this)
    , input_method_manager_v2_input_method_(this)
    , input_method_v2_destroy_(this)
    , text_input_manager_v3_text_input_(this)
    , x11ActiveWindow_(this)
    , textInputCursorRectangle_(this)
    , output_present_(this)
{
    if (getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET")) {
        sessionType_ = SessionType ::WL;
        backend_.reset(wlr_wl_backend_create(display_.get(), nullptr));
        wlr_wl_output_create(backend_.get());
    } else if (const char *display = getenv("DISPLAY")) {
        sessionType_ = SessionType ::X11;
        backend_.reset(wlr_x11_backend_create(display_.get(), display));
        wlr_x11_output_create(backend_.get());

        unsafe_wlr_x11_backend *x11_backend = wl_container_of(backend_.get(), x11_backend, backend);
        xcb_helper_.setConnection(x11_backend->xcb);
    }

    if (!backend_) {
        throw std::runtime_error("failed to create backend");
    }

    renderer_.reset(wlr_renderer_autocreate(backend_.get()));
    if (!renderer_) {
        throw std::runtime_error("failed to create wlr_renderer");
    }

    wlr_renderer_init_wl_display(renderer_.get(), display_.get());

    allocator_.reset(wlr_allocator_autocreate(backend_.get(), renderer_.get()));
    if (!allocator_) {
        throw std::runtime_error("failed to create wlr_allocator");
    }

    /* This creates some hands-off wlroots interfaces. The compositor is
     * necessary for clients to allocate surfaces, the subcompositor allows to
     * assign the role of subsurfaces to surfaces and the data device manager
     * handles the clipboard. Each of these wlroots interfaces has room for you
     * to dig your fingers in and play with their behavior if you want. Note that
     * the clients cannot set the selection directly without compositor approval,
     * see the handling of the request_set_selection event below.*/
#if WLR_VERSION_MINOR >= 17
    wlr_compositor_create(display_.get(), 5, renderer_.get());
#else
    wlr_compositor_create(display_.get(), renderer_.get());
#endif
    wlr_subcompositor_create(display_.get());
    wlr_data_device_manager_create(display_.get());

    /* Creates an output layout, which a wlroots utility for working with an
     * arrangement of screens in a physical layout. */
    output_layout_.reset(wlr_output_layout_create());

    /* Configure a listener to be notified when new outputs are available on the
     * backend. */
    wl_signal_add(&backend_->events.new_output, backend_new_output_);

    /* Create a scene graph. This is a wlroots abstraction that handles all
     * rendering and damage tracking. All the compositor author needs to do
     * is add things that should be rendered to the scene graph at the proper
     * positions and then call wlr_scene_output_commit() to render a frame if
     * necessary.
     */
    scene_.reset(wlr_scene_create());
    wlr_scene_attach_output_layout(scene_.get(), output_layout_.get());

    /* Set up xdg-shell version 3. The xdg-shell is a Wayland protocol which is
     * used for application windows. For more detail on shells, refer to my
     * article:
     *
     * https://drewdevault.com/2018/07/29/Wayland-shells.html
     */
    wl_list_init(&views_);
    xdg_shell_.reset(wlr_xdg_shell_create(display_.get(), 3));
    wl_signal_add(&xdg_shell_->events.new_surface, xdg_shell_new_surface_);

    /*
     * Creates a cursor, which is a wlroots utility for tracking the cursor
     * image shown on screen.
     */
    cursor_.reset(wlr_cursor_create());
    wlr_cursor_attach_output_layout(cursor_.get(), output_layout_.get());

    /* Creates an xcursor manager, another wlroots utility which loads up
     * Xcursor themes to source cursor images from and makes sure that cursor
     * images are available at all scale factors on the screen (necessary for
     * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
    cursor_mgr_.reset(wlr_xcursor_manager_create(nullptr, 24));
    wlr_xcursor_manager_load(cursor_mgr_.get(), 1);

    /*
     * wlr_cursor *only* displays an image on screen. It does not move around
     * when the pointer moves. However, we can attach input devices to it, and
     * it will generate aggregate events for all of them. In these events, we
     * can choose how we want to process them, forwarding them to clients and
     * moving the cursor around. More detail on this process is described in my
     * input handling blog post:
     *
     * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
     *
     * And more comments are sprinkled throughout the notify functions above.
     */
    // server.cursor_mode = TINYWL_CURSOR_PASSTHROUGH;
    wl_signal_add(&cursor_->events.motion, cursor_motion_);
    wl_signal_add(&cursor_->events.motion_absolute, cursor_motion_absolute_);
    wl_signal_add(&cursor_->events.button, cursor_button_);
    wl_signal_add(&cursor_->events.axis, cursor_axis_);
    wl_signal_add(&cursor_->events.frame, cursor_frame_);

    /*
     * Configures a seat, which is a single "seat" at which a user sits and
     * operates the computer. This conceptually includes up to one keyboard,
     * pointer, touch, and drawing tablet device. We also rig up a listener to
     * let us know when new input devices are available on the backend.
     */
    seat_.reset(wlr_seat_create(display_.get(), "seat0"));
    wl_signal_add(&seat_->events.request_set_cursor, seat_request_cursor_);
    wl_signal_add(&seat_->events.request_set_selection, seat_request_set_selection_);
    wl_list_init(&keyboards_);
    wl_signal_add(&backend_->events.new_input, backend_new_input_);

    virtual_keyboard_manager_v1_.reset(wlr_virtual_keyboard_manager_v1_create(display_.get()));
    wl_signal_add(&virtual_keyboard_manager_v1_->events.new_virtual_keyboard,
                  virtual_keyboard_manager_v1_new_virtual_keyboard_);

    input_method_manager_v2_.reset(wlr_input_method_manager_v2_create(display_.get()));
    wl_signal_add(&input_method_manager_v2_->events.input_method,
                  input_method_manager_v2_input_method_);

    wl_list_init(&text_inputs_);
    text_input_manager_v3_.reset(wlr_text_input_manager_v3_create(display_.get()));
    wl_signal_add(&text_input_manager_v3_->events.text_input, text_input_manager_v3_text_input_);

    if (sessionType_ == SessionType::X11) {
        struct wl_event_loop *loop = wl_display_get_event_loop(display_.get());
        x11ActiveWindowMonitor_ = std::make_unique<X11ActiveWindowMonitor>(loop);

        wl_signal_add(&x11ActiveWindowMonitor_->events.activeWindow, x11ActiveWindow_);
    }
}

Server::~Server() = default;

std::string Server::addSocketAuto()
{
    return wl_display_add_socket_auto(display_.get());
}

bool Server::addSocket(const std::string &name)
{
    return wl_display_add_socket(display_.get(), name.c_str()) == 0;
}

void Server::run()
{
    wl_display_run(display_.get());
}

bool Server::startBackend()
{
    return wlr_backend_start(backend_.get());
}

wl_event_loop *Server::getLoop()
{
    return wl_display_get_event_loop(display_.get());
}

void Server::setTextInputFocus(wlr_surface *surface)
{
    if (!input_method_) {
        return;
    }

    TextInputV3 *textInput;
    wl_list_for_each(textInput, &text_inputs_, link_)
    {
        if (textInput->text_input_->focused_surface) {
            if (surface != textInput->text_input_->focused_surface) {
                textInput->disableTextInput(input_method_);
                wlr_text_input_v3_send_leave(textInput->text_input_);
            } else {
                continue;
            }
        }

        if (surface
            && wl_resource_get_client(textInput->text_input_->resource)
                == wl_resource_get_client(surface->resource)) {
            wlr_text_input_v3_send_enter(textInput->text_input_, surface);
        }
    }
}

void Server::backendNewOutputNotify(void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    assert(output_ == nullptr);
    struct wlr_output *output = static_cast<struct wlr_output *>(data);

    wlr_output_set_custom_mode(output, 1, 1, 0);
    wl_signal_add(&output->events.present, output_present_);

    if (sessionType_ == SessionType::X11) {
        unsafe_wlr_x11_output *x11_output = wl_container_of(output, x11_output, wlr_output);

        xcb_helper_.flush();
    }

    output_ = new Output(this, output);
}

void Server::outputPresentNotify(void *data)
{
    if (sessionType_ == SessionType::X11) {
        unsafe_wlr_x11_output *x11_output = wl_container_of(output_, x11_output, wlr_output);
        xcb_helper_.unmapWindow(x11_output->win);
        xcb_helper_.setPropertyAtom(x11_output->win,
                                    "_NET_WM_WINDOW_TYPE",
                                    "_NET_WM_WINDOW_TYPE_POPUP_MENU");
        xcb_helper_.flush();
    }
}

void Server::xdgShellNewSurfaceNotify(void *data)
{
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    struct wlr_xdg_surface *xdg_surface = static_cast<wlr_xdg_surface *>(data);

    /* We must add xdg popups to the scene graph so they get rendered. The
     * wlroots scene graph provides a helper for this, but to use it we must
     * provide the proper parent scene node of the xdg popup. To enable this,
     * we always set the user data field of xdg_surfaces to the corresponding
     * scene node. */
    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        struct wlr_xdg_surface *parent =
            wlr_xdg_surface_try_from_wlr_surface(xdg_surface->popup->parent);
        struct wlr_scene_tree *parent_tree = static_cast<wlr_scene_tree *>(parent->data);
        xdg_surface->data = wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
        return;
    }
    assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

    new View(this, xdg_surface, &views_);
}

void Server::cursorMotionNotify(void *data)
{
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    struct wlr_pointer_motion_event *event = static_cast<wlr_pointer_motion_event *>(data);
    /* The cursor doesn't move unless we tell it to. The cursor automatically
     * handles constraining the motion to the output layout, as well as any
     * special configuration applied for the specific input device which
     * generated the event. You can pass NULL for the device if you want to move
     * the cursor around without any input. */
    wlr_cursor_move(cursor_.get(), &event->pointer->base, event->delta_x, event->delta_y);
    // process_cursor_motion(server, event->time_msec);
}

void Server::cursorMotionAbsoluteNotify(void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an _absolute_
     * motion event, from 0..1 on each axis. This happens, for example, when
     * wlroots is running under a Wayland window rather than KMS+DRM, and you
     * move the mouse over the window. You could enter the window from any edge,
     * so we have to warp the mouse there. There is also some hardware which
     * emits these events. */
    struct wlr_pointer_motion_absolute_event *event =
        static_cast<wlr_pointer_motion_absolute_event *>(data);
    wlr_cursor_warp_absolute(cursor_.get(), &event->pointer->base, event->x, event->y);
    processCursorMotion(event->time_msec);
}

void Server::cursorButtonNotify(void *data)
{
    /* This event is forwarded by the cursor when a pointer emits a button
     * event. */
    struct wlr_pointer_button_event *event = static_cast<wlr_pointer_button_event *>(data);
    /* Notify the client with pointer focus that a button press has occurred */
    wlr_seat_pointer_notify_button(seat_.get(), event->time_msec, event->button, event->state);
    double sx, sy;
    struct wlr_surface *surface = NULL;
    View *view = desktopViewAt(cursor_->x, cursor_->y, &surface, &sx, &sy);
    if (event->state == WLR_BUTTON_RELEASED) {
        /* If you released any buttons, we exit interactive move/resize mode. */
        // reset_cursor_mode(server);
    } else {
        /* Focus that client if the button was _pressed_ */
        // focus_view(view, surface);
    }
}

void Server::cursorAxisNotify(void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an axis event,
     * for example when you move the scroll wheel. */
    struct wlr_pointer_axis_event *event = static_cast<wlr_pointer_axis_event *>(data);
    /* Notify the client with pointer focus of the axis event. */
    wlr_seat_pointer_notify_axis(seat_.get(),
                                 event->time_msec,
                                 event->orientation,
                                 event->delta,
                                 event->delta_discrete,
                                 event->source);
}

void Server::cursorFrameNotify(void *data)
{
    /* This event is forwarded by the cursor when a pointer emits an frame
     * event. Frame events are sent after regular pointer events to group
     * multiple events together. For instance, two axis events may happen at the
     * same time, in which case a frame event won't be sent in between. */
    /* Notify the client with pointer focus of the frame event. */
    wlr_seat_pointer_notify_frame(seat_.get());
}

void Server::backendNewInputNotify(void *data)
{
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct wlr_input_device *device = static_cast<wlr_input_device *>(data);
    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        new Keyboard(this, device, &keyboards_);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        /* We don't do anything special with pointers. All of our pointer handling
         * is proxied through wlr_cursor. On another compositor, you might take this
         * opportunity to do libinput configuration on the device to set
         * acceleration, etc. */
        wlr_cursor_attach_input_device(cursor_.get(), device);
        break;
    default:
        break;
    }
    /* We need to let the wlr_seat know what our capabilities are, which is
     * communiciated to the client. In TinyWL we always have a cursor, even if
     * there are no pointer devices, so we always include that capability. */
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&keyboards_)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(seat_.get(), caps);
}

void Server::seatRequestCursorNotify(void *data) { }

void Server::seatRequestSetSelectionNotify(void *data) { }

void Server::virtualKeyboardManagerNewVirtualKeyboardNotify(void *data)
{
    struct wlr_virtual_keyboard_v1 *keyboard = static_cast<wlr_virtual_keyboard_v1 *>(data);
    struct wlr_input_device *device = &keyboard->keyboard.base;

    new Keyboard(this, device, &keyboards_);
}

void Server::inputMethodManagerV2InputMethodNotify(void *data)
{
    if (input_method_) {
        return;
    }

    auto *im2 = static_cast<wlr_input_method_v2 *>(data);
    input_method_ = new InputMethodV2(this, im2);
    wl_signal_add(&im2->events.destroy, input_method_v2_destroy_);
}

void Server::inputMethodV2DestroyNotify(void *data)
{
    input_method_ = nullptr;
}

void Server::textInputManagerV3TextInputNotify(void *data)
{
    auto *ti3 = static_cast<wlr_text_input_v3 *>(data);

    auto *textInput = new TextInputV3(this, ti3, &text_inputs_);
    wl_signal_add(&textInput->events.cursorRectangle, textInputCursorRectangle_);
}

void Server::textInputV3DestroyNotify(void *data) { }

void Server::x11ActiveWindowNotify(void *data)
{
    xcb_window_t win = *static_cast<xcb_window_t *>(data);

    pid_t pid = x11ActiveWindowMonitor_->windowPid(win);
    if (!pid) {
        return;
    }

    wlr_surface *focusedSurface = nullptr;
    View *view = nullptr;
    wl_list_for_each(view, &views_, link_)
    {
        if (view->getPid() == pid) {
            view->focusView();
            focusedSurface = view->surface();
        }
    }

    setTextInputFocus(focusedSurface);
}

void Server::textInputCursorRectangleNotify(void *data)
{
    auto *rectangle = static_cast<wlr_box *>(data);
    if (input_method_) {
        input_method_->setCursorRectangle(rectangle);
    }

    // if (sessionType_ == SessionType::X11) {
    //     auto [x, y] =
    //         x11ActiveWindowMonitor_->windowPosition(x11ActiveWindowMonitor_->activeWindow());

    //     unsafe_wlr_x11_output *x11_output =
    //         wl_container_of(output_->output(), x11_output, wlr_output);
    //     xcb_params_configure_window_t wc;
    //     wc.x = x + rectangle->x + rectangle->height;
    //     wc.y = y + rectangle->y + rectangle->width;
    //     wc.stack_mode = XCB_STACK_MODE_ABOVE;
    //     xcb_helper_.auxConfigureWindow(x11_output->win,
    //                                    XCB_CONFIG_WINDOW_STACK_MODE | XCB_CONFIG_WINDOW_X
    //                                        | XCB_CONFIG_WINDOW_Y,
    //                                    &wc);
    //     xcb_helper_.flush();
    // }
}

void Server::processCursorMotion(uint32_t time)
{
    /* If the mode is non-passthrough, delegate to those functions. */
    // if (cursor_mode == TINYWL_CURSOR_MOVE) {
    //     process_cursor_move(server, time);
    //     return;
    // } else if (server->cursor_mode == TINYWL_CURSOR_RESIZE) {
    //     process_cursor_resize(server, time);
    //     return;
    // }

    /* Otherwise, find the view under the pointer and send the event along. */
    double sx, sy;
    struct wlr_seat *seat = seat_.get();
    struct wlr_surface *surface = NULL;
    View *view = desktopViewAt(cursor_->x, cursor_->y, &surface, &sx, &sy);
    if (!view) {
        /* If there's no toplevel under the cursor, set the cursor image to a
         * default. This is what makes the cursor image appear when you move it
         * around the screen, not over any toplevels. */
#if WLR_VERSION_MINOR >= 17
        wlr_cursor_set_xcursor(cursor_.get(), cursor_mgr_.get(), "default");
#else
        wlr_xcursor_manager_set_cursor_image(cursor_mgr_.get(), "left_ptr", cursor_.get());
#endif
    }
    if (surface) {
        /*
         * Send pointer enter and motion events.
         *
         * The enter event gives the surface "pointer focus", which is distinct
         * from keyboard focus. You get pointer focus by moving the pointer over
         * a window.
         *
         * Note that wlroots will avoid sending duplicate enter/motion events if
         * the surface has already has pointer focus or if the client is already
         * aware of the coordinates passed.
         */
        wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(seat, time, sx, sy);
    } else {
        /* Clear pointer focus so future button events and such are not sent to
         * the last client to have the cursor over it. */
        wlr_seat_pointer_clear_focus(seat);
    }
}

View *
Server::desktopViewAt(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy)
{
    /* This returns the topmost node in the scene at the given layout coords.
     * We only care about surface nodes as we are specifically looking for a
     * surface in the surface tree of a toplevel. */
    struct wlr_scene_node *node = wlr_scene_node_at(&scene_->tree.node, lx, ly, sx, sy);
    if (node == nullptr || node->type != WLR_SCENE_NODE_BUFFER) {
        return nullptr;
    }
    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);
    if (!scene_surface) {
        return nullptr;
    }

    *surface = scene_surface->surface;
    /* Find the node corresponding to the toplevel at the root of this
     * surface tree, it is the only one for which we set the data field. */
    struct wlr_scene_tree *tree = node->parent;
    while (tree != nullptr && tree->node.data == nullptr) {
        tree = tree->node.parent;
    }
    return static_cast<View *>(tree->node.data);
}

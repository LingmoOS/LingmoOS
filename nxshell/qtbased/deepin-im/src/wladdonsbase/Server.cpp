// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Server.h"

#include "InputMethodContextV1.h"
#include "InputMethodV1.h"
#include "InputMethodV2.h"
#include "Keyboard.h"
#include "Output.h"
#include "View.h"
#include "inputmethodv1/ZwpInputMethodV1.h"

#include <experimental/unordered_map>

#include <stdexcept>

extern "C" {
#include <wlr/backend/wayland.h>
#include <wlr/backend/x11.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#define delete delete_
#include <wlr/types/wlr_input_method_v2.h>
#undef delete
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/util/log.h>
#include <wlr/version.h>
}

#include <assert.h>

#if WLR_VERSION_MINOR < 17
#  define wlr_xdg_surface_try_from_wlr_surface(SURFACE) wlr_xdg_surface_from_wlr_surface(SURFACE)
#  define wlr_scene_surface_try_from_buffer(BUFFER) wlr_scene_surface_from_buffer(BUFFER)
#endif

WL_ADDONS_BASE_USE_NAMESPACE

Server::Server(const std::shared_ptr<wl_display> &local,
               const std::shared_ptr<wlr_backend> &backend)
    : display_(local)
    , backend_(backend)
    , backend_new_output_(this)
    , compositor_new_surface_(this)
    , xdg_shell_new_surface_(this)
    , seat_request_cursor_(this)
    , seat_request_set_selection_(this)
    , backend_new_input_(this)
    , virtual_keyboard_manager_v1_new_virtual_keyboard_(this)
    , input_method_manager_v2_input_method_(this)
    , input_method_v2_destroy_(this)
    , output_present_(this)
{
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
    auto *compositor = wlr_compositor_create(display_.get(), 5, renderer_.get());
#else
    auto *compositor = wlr_compositor_create(display_.get(), renderer_.get());
#endif
    wl_signal_add(&compositor->events.new_surface, compositor_new_surface_);

    wlr_subcompositor_create(display_.get());
    wlr_data_device_manager_create(display_.get());

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
    inputMethodV1_.reset(new InputMethodV1(this));
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

int Server::getFd()
{
    return wl_display_get_fd(display_.get());
}

wl_event_loop *Server::getEventLoop()
{
    return wl_display_get_event_loop(display_.get());
}

void Server::flushClients()
{
    wl_display_flush_clients(display_.get());
}

void Server::run()
{
    wl_display_run(display_.get());
}

void Server::createOutput()
{
    wlr_wl_output_create(backend_.get());
}

void Server::createOutputFromSurface(wl_surface *surface)
{
    wlr_wl_output_create_from_surface(backend_.get(), surface);
}

void Server::backendNewOutputNotify(void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    assert(output_ == nullptr);
    struct wlr_output *output = static_cast<struct wlr_output *>(data);

    wl_signal_add(&output->events.present, output_present_);

    output_ = new Output(this, output);
}

void Server::outputPresentNotify(void *data) { }

void Server::compositorNewSurfaceNotify(void *data)
{
    struct wlr_surface *surface = static_cast<struct wlr_surface *>(data);
    wlr_scene_surface_create(&scene_->tree, surface);
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

void Server::backendNewInputNotify(void *data)
{
    /* This event is raised by the backend when a new input device becomes
     * available. */
    struct wlr_input_device *device = static_cast<wlr_input_device *>(data);
    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD: {
        auto *keyboard = new Keyboard(this, device, &keyboards_);
    } break;
    case WLR_INPUT_DEVICE_POINTER:
        /* We don't do anything special with pointers. All of our pointer handling
         * is proxied through wlr_cursor. On another compositor, you might take this
         * opportunity to do libinput configuration on the device to set
         * acceleration, etc. */
        // wlr_cursor_attach_input_device(cursor_.get(), device);
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

void Server::keyboardKeyNotify(void *data) { }

void Server::virtualKeyboardManagerNewVirtualKeyboardNotify(void *data)
{
    struct wlr_virtual_keyboard_v1 *keyboard = static_cast<wlr_virtual_keyboard_v1 *>(data);
    struct wlr_input_device *device = &keyboard->keyboard.base;

    auto *kbd = new Keyboard(this, device, &keyboards_, true);
    virtualKeyboardCallback_(kbd);
}

void Server::inputMethodManagerV2InputMethodNotify(void *data)
{
    auto *im2 = static_cast<wlr_input_method_v2 *>(data);
    std::string path = getExePathByPid(getPid(im2->resource));

    IMType imType = IMType::FCITX5;
    if (endsWith(path, "fcitx5")) {
        imType = IMType::FCITX5;
    } else if (endsWith(path, "ibus-daemon")) {
        imType = IMType::IBUS;
    } else {
        // unknown
        return;
    }

    auto im = std::make_unique<InputMethodV2>(this, im2);
    inputMethodV2s_.emplace(imType, std::move(im));
    wl_signal_add(&im2->events.destroy, input_method_v2_destroy_);

    if (inputMethodCallback_) {
        inputMethodCallback_();
    }
}

void Server::inputMethodV2DestroyNotify(void *data)
{
    auto *wlrIM2 = static_cast<wlr_input_method_v2 *>(data);
    std::experimental::erase_if(inputMethodV2s_, [wlrIM2](const auto &item) {
        const auto &[key, im2] = item;
        if (im2->is(wlrIM2)) {
            return true;
        }

        return false;
    });
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

InputMethodContextV1 *Server::inputMethodContextV1() const
{
    return inputMethodV1_->inputMethodContextV1().get();
}

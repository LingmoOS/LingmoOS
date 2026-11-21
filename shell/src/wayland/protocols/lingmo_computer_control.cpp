// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/lingmo_computer_control.h"

#include "base/strings.h"

#include <cerrno>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

#include <lingmo-computer-control-v1-server-protocol.h>
#include <wayland-server-protocol.h>

#include "computer_control/controller.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {
namespace {

using computer_control::Control;
using computer_control::Controller;
using computer_control::Rejection;
using computer_control::RevokeReason;
using computer_control::WindowInfo;

struct ControlResource final : computer_control::ControlClient {
    explicit ControlResource(wl_resource *resource)
        : resource(resource)
    {
    }

    void granted() override { lingmo_computer_control_v1_send_granted(resource); }

    void revoked(RevokeReason reason) override
    {
        lingmo_computer_control_v1_send_revoked(resource, static_cast<uint32_t>(reason));
    }

    void rejected(Rejection rejection) override
    {
        lingmo_computer_control_v1_send_rejected(resource, static_cast<uint32_t>(rejection));
    }

    void window(uint32_t handle, const WindowInfo &window) override
    {
        lingmo_computer_control_v1_send_window(
                resource, handle, window.applicationId.c_str(), window.title.c_str(),
                window.bounds.x(), window.bounds.y(),
                window.bounds.width(), window.bounds.height(), window.flags);
    }

    void windowsDone() override { lingmo_computer_control_v1_send_windows_done(resource); }

    void pointer(const base::PointF &position) override
    {
        lingmo_computer_control_v1_send_pointer(resource, wl_fixed_from_double(position.x()),
                                                  wl_fixed_from_double(position.y()));
    }

    wl_resource *const resource;
    std::unique_ptr<Control> control;
};

Control *controlOf(wl_resource *resource)
{
    return getUserDataAs<ControlResource>(resource)->control.get();
}

// Returns the state as a press, or nothing after posting the error.
std::optional<bool> pressed(wl_resource *resource, uint32_t state)
{
    if (state > LINGMO_COMPUTER_CONTROL_V1_STATE_PRESSED) {
        wl_resource_post_error(resource, LINGMO_COMPUTER_CONTROL_V1_ERROR_INVALID_STATE,
                               "state must be released or pressed");
        return std::nullopt;
    }
    return state == LINGMO_COMPUTER_CONTROL_V1_STATE_PRESSED;
}

// Only a regular file is read at once, so the client cannot keep the
// compositor waiting.
constexpr off_t kClipboardLimit = 16 * 1024 * 1024;

// Returns the text, or nothing after posting the error. Closes the fd.
std::optional<std::string> readClipboard(wl_resource *resource, int fd)
{
    struct stat info {};
    if (fstat(fd, &info) < 0 || !S_ISREG(info.st_mode) || info.st_size > kClipboardLimit) {
        close(fd);
        wl_resource_post_error(resource, LINGMO_COMPUTER_CONTROL_V1_ERROR_INVALID_CLIPBOARD,
                               "the fd must be a regular file of at most 16 MiB");
        return std::nullopt;
    }
    std::string text(static_cast<size_t>(info.st_size), '\0');
    size_t done = 0;
    while (done < text.size()) {
        const ssize_t count = pread(fd, text.data() + done, text.size() - done,
                                    static_cast<off_t>(done));
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            break;
        done += static_cast<size_t>(count);
    }
    close(fd);
    text.resize(done);
    return text;
}

void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

// Nulled when the client destroys the wl_callback before the answer.
struct CallbackOwner {
    explicit CallbackOwner(std::shared_ptr<wl_resource *> token)
        : token(std::move(token))
    {
    }
    ~CallbackOwner() { *token = nullptr; }

    std::shared_ptr<wl_resource *> token;
};

void settle(wl_client *client, wl_resource *resource, uint32_t id, uint32_t timeout)
{
    wl_resource *callback = wl_resource_create(client, &wl_callback_interface, 1, id);
    if (!callback) {
        wl_client_post_no_memory(client);
        return;
    }
    auto token = std::make_shared<wl_resource *>(callback);
    setImplementation(callback, nullptr, std::make_unique<CallbackOwner>(token));
    controlOf(resource)->settle(std::chrono::milliseconds(timeout), [token](bool settled) {
        if (wl_resource *callback = *token) {
            wl_callback_send_done(callback, settled ? 1 : 0);
            wl_resource_destroy(callback);
        }
    });
}

const struct lingmo_computer_control_v1_interface controlImplementation = {
        destroy,
        [](wl_client *, wl_resource *resource, wl_fixed_t x, wl_fixed_t y) {
            controlOf(resource)->motion(
                    base::PointF(wl_fixed_to_double(x), wl_fixed_to_double(y)));
        },
        [](wl_client *, wl_resource *resource, uint32_t button, uint32_t state) {
            if (const auto press = pressed(resource, state))
                controlOf(resource)->button(button, *press);
        },
        [](wl_client *, wl_resource *resource, int32_t horizontal, int32_t vertical) {
            controlOf(resource)->axis(horizontal, vertical);
        },
        [](wl_client *, wl_resource *resource, uint32_t key, uint32_t state) {
            if (const auto press = pressed(resource, state))
                controlOf(resource)->key(key, *press);
        },
        [](wl_client *, wl_resource *resource, uint32_t keysym, uint32_t state) {
            if (const auto press = pressed(resource, state))
                controlOf(resource)->keysym(keysym, *press);
        },
        [](wl_client *, wl_resource *resource, const char *text) {
            controlOf(resource)->typeText(text);
        },
        [](wl_client *, wl_resource *resource) {
            controlOf(resource)->listWindows();
        },
        [](wl_client *, wl_resource *resource, uint32_t handle) {
            controlOf(resource)->activateWindow(handle);
        },
        [](wl_client *, wl_resource *resource) {
            controlOf(resource)->queryPointer();
        },
        [](wl_client *, wl_resource *resource, uint32_t handle) {
            controlOf(resource)->closeWindow(handle);
        },
        [](wl_client *, wl_resource *resource, uint32_t handle, int32_t x, int32_t y,
           int32_t width, int32_t height) {
            controlOf(resource)->setWindowFrame(handle, clientRect(x, y, width, height));
        },
        [](wl_client *, wl_resource *resource, int32_t fd) {
            if (const auto text = readClipboard(resource, fd))
                controlOf(resource)->setClipboard(*text);
        },
        settle,
};

void requestControl(wl_client *client, wl_resource *manager, uint32_t id, const char *agentName)
{
    auto *controller = getUserDataAs<Controller>(manager);
    wl_resource *resource = wl_resource_create(client, &lingmo_computer_control_v1_interface,
                                               wl_resource_get_version(manager), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto *control = new ControlResource(resource);
    wl_resource_set_implementation(resource, &controlImplementation, control,
                                   [](wl_resource *resource) {
                                       delete getUserDataAs<ControlResource>(resource);
                                   });
    pid_t pid = 0;
    wl_client_get_credentials(client, &pid, nullptr, nullptr);
    control->control = controller->request(control, client,
                                           base::strings::sanitizedUtf8(agentName ? agentName : ""),
                                           pid);
}

const struct lingmo_computer_control_manager_v1_interface managerImplementation = {
        destroy,
        requestControl,
};

} // namespace

WaylandComputerControl::WaylandComputerControl(wl_display *display, Controller *controller,
                                               std::function<bool(const wl_client *)> authorize)
    : m_controller(controller)
    , m_authorize(std::move(authorize))
{
    m_global = wl_global_create(display, &lingmo_computer_control_manager_v1_interface, 1, this,
                                bind);
}

WaylandComputerControl::~WaylandComputerControl()
{
    if (m_global)
        wl_global_destroy(m_global);
}

void WaylandComputerControl::bind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *self = static_cast<WaylandComputerControl *>(data);
    if (!self->m_authorize(client)) {
        wl_client_post_implementation_error(client, "Computer control not authorized");
        return;
    }
    wl_resource *resource =
            wl_resource_create(client, &lingmo_computer_control_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, self->m_controller, nullptr);
}

} // namespace Lingmo::Wayland

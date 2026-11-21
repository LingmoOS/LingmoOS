// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <chrono>
#include <fcntl.h>
#include <functional>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wayland-client.h>

#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/scene.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_data_device_manager.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server.h"
#include "wayland/server_util.h"

namespace clipboard_test {

using namespace Lingmo;

struct Client {
    wl_display* display = nullptr;
    wl_client* serverClient = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* compositor = nullptr;
    wl_seat* seat = nullptr;
    wl_keyboard* keyboard = nullptr;
    wl_pointer* pointer = nullptr;
    wl_touch* touch = nullptr;
    wl_data_offer* dragOffer = nullptr;
    std::vector<std::pair<uint32_t, bool>> keyEvents;
    uint32_t buttonSerial = 0;
    size_t pointerEnters = 0;
    size_t pointerLeaves = 0;
    size_t pointerButtons = 0;
    uint32_t touchSerial = 0;
    size_t touchCancels = 0;
    uint32_t dragSerial = 0;
    uint32_t dragAction = 0;
    uint32_t sourceAction = 0;
    size_t drops = 0;
    size_t finished = 0;
    size_t performed = 0;
    wl_data_device_manager* manager = nullptr;
    wl_surface* surface = nullptr;
    uint32_t serial = 0;
    std::vector<wl_data_device*> devices;
    std::vector<wl_data_source*> sources;
    std::vector<wl_data_offer*> offers;
    std::map<wl_data_offer*, std::vector<std::string>> formats;
    wl_data_offer* selection = nullptr;
    std::vector<std::string> events;
    Lingmo::base::PointF dragEnterPosition;
    Lingmo::base::PointF dragMotionPosition;
    std::map<std::string, std::string> payloads;
    size_t cancelled = 0;
    size_t sends = 0;
    bool stall = false;
    bool expectError = false;
    uint32_t version = 3;

    struct Write {
        int fd;
        std::string bytes;
        size_t offset = 0;
    };

    std::vector<Write> writes;

    // `connect` turns the server end of the socket into a wl_client, as the
    // privileged shell connection is made; wl_client_create when empty.
    explicit Client(wayland::Server& server, uint32_t version = 3,
                    const std::function<wl_client*(int)>& connect = {})
        : version(version)
    {
        int fds[2];
        EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds));
        serverClient = connect ? connect(fds[0]) : wl_client_create(server.display(), fds[0]);
        if (!serverClient) {
            close(fds[1]);
            throw std::runtime_error("wl_client_create failed");
        }
        display = wl_display_connect_to_fd(fds[1]);
        registry = wl_display_get_registry(display);
        static const wl_registry_listener listener = {
            [](void* data, wl_registry* registry, uint32_t name, const char* interface,
               uint32_t version) {
                auto& c = *static_cast<Client*>(data);
                const std::string type(interface);
                if (type == "wl_compositor")
                    c.compositor = static_cast<wl_compositor*>(
                        wl_registry_bind(registry, name, &wl_compositor_interface, 4));
                if (type == "wl_seat") {
                    if (c.seat)
                        wl_seat_release(c.seat);
                    c.seat = static_cast<wl_seat*>(
                        wl_registry_bind(registry, name, &wl_seat_interface, 7));
                }
                if (type == "wl_data_device_manager")
                    c.manager = static_cast<wl_data_device_manager*>(
                        wl_registry_bind(registry, name, &wl_data_device_manager_interface,
                                         std::min(version, c.version)));
            },
            [](void*, wl_registry*, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
    }

    void initialize(bool device = true)
    {
        ASSERT_TRUE(compositor);
        ASSERT_TRUE(seat);
        ASSERT_TRUE(manager);
        surface = wl_compositor_create_surface(compositor);
        keyboard = wl_seat_get_keyboard(seat);
        static const wl_keyboard_listener keys = {
            [](void*, wl_keyboard*, uint32_t, int fd, uint32_t) {
                close(fd);
            },
            [](void* data, wl_keyboard*, uint32_t serial, wl_surface*, wl_array*) {
                auto& c = *static_cast<Client*>(data);
                c.serial = serial;
                c.events.push_back("enter");
            },
            [](void* data, wl_keyboard*, uint32_t, wl_surface*) {
                static_cast<Client*>(data)->events.push_back("leave");
            },
            [](void* data, wl_keyboard*, uint32_t serial, uint32_t, uint32_t key, uint32_t state) {
                auto* client = static_cast<Client*>(data);
                client->serial = serial;
                client->keyEvents.emplace_back(key, state == WL_KEYBOARD_KEY_STATE_PRESSED);
            },
            [](void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {},
            [](void*, wl_keyboard*, int32_t, int32_t) {}};
        wl_keyboard_add_listener(keyboard, &keys, this);
        pointer = wl_seat_get_pointer(seat);
        static const wl_pointer_listener pointerListener = {
            [](void* data, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t) {
                ++static_cast<Client*>(data)->pointerEnters;
            },
            [](void* data, wl_pointer*, uint32_t, wl_surface*) {
                ++static_cast<Client*>(data)->pointerLeaves;
            },
            [](void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t) {},
            [](void* data, wl_pointer*, uint32_t serial, uint32_t, uint32_t, uint32_t state) {
                auto* client = static_cast<Client*>(data);
                ++client->pointerButtons;
                if (state == WL_POINTER_BUTTON_STATE_PRESSED)
                    client->buttonSerial = serial;
            },
            [](void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t) {},
            [](void*, wl_pointer*) {},
            [](void*, wl_pointer*, uint32_t) {},
            [](void*, wl_pointer*, uint32_t, uint32_t) {},
            [](void*, wl_pointer*, uint32_t, int32_t) {}};
        wl_pointer_add_listener(pointer, &pointerListener, this);
        if (device)
            addDevice();
    }

    void getTouch()
    {
        touch = wl_seat_get_touch(seat);
        static const wl_touch_listener touchListener = {
            [](void* data, wl_touch*, uint32_t serial, uint32_t, wl_surface*, int32_t, wl_fixed_t,
               wl_fixed_t) {
                static_cast<Client*>(data)->touchSerial = serial;
            },
            [](void*, wl_touch*, uint32_t, uint32_t, int32_t) {},
            [](void*, wl_touch*, uint32_t, int32_t, wl_fixed_t, wl_fixed_t) {},
            [](void*, wl_touch*) {},
            [](void* data, wl_touch*) {
                ++static_cast<Client*>(data)->touchCancels;
            },
            nullptr,
            nullptr};
        wl_touch_add_listener(touch, &touchListener, this);
    }

    void addDevice()
    {
        auto* device = wl_data_device_manager_get_data_device(manager, seat);
        devices.push_back(device);
        static const wl_data_offer_listener offerListener = {
            [](void* data, wl_data_offer* offer, const char* mime) {
                auto& c = *static_cast<Client*>(data);
                c.formats[offer].emplace_back(mime);
                c.events.push_back("mime");
            },
            [](void*, wl_data_offer*, uint32_t) {},
            [](void* data, wl_data_offer*, uint32_t action) {
                static_cast<Client*>(data)->dragAction = action;
            }};
        static const wl_data_device_listener listener = {
            [](void* data, wl_data_device*, wl_data_offer* offer) {
                auto& c = *static_cast<Client*>(data);
                c.offers.push_back(offer);
                c.events.push_back("offer");
                wl_data_offer_add_listener(offer, &offerListener, data);
            },
            [](void* data, wl_data_device*, uint32_t serial, wl_surface*, wl_fixed_t x,
               wl_fixed_t y, wl_data_offer* offer) {
                auto& c = *static_cast<Client*>(data);
                c.dragOffer = offer;
                c.dragSerial = serial;
                c.dragEnterPosition = Lingmo::base::PointF(wl_fixed_to_double(x), wl_fixed_to_double(y));
                c.events.push_back("drag-enter");
            },
            [](void* data, wl_data_device*) {
                static_cast<Client*>(data)->events.push_back("drag-leave");
            },
            [](void* data, wl_data_device*, uint32_t, wl_fixed_t x, wl_fixed_t y) {
                static_cast<Client*>(data)->dragMotionPosition =
                    Lingmo::base::PointF(wl_fixed_to_double(x), wl_fixed_to_double(y));
                static_cast<Client*>(data)->events.push_back("drag-motion");
            },
            [](void* data, wl_data_device*) {
                ++static_cast<Client*>(data)->drops;
            },
            [](void* data, wl_data_device*, wl_data_offer* offer) {
                auto& c = *static_cast<Client*>(data);
                c.selection = offer;
                c.events.push_back(offer ? "selection" : "null");
            }};
        wl_data_device_add_listener(device, &listener, this);
    }

    wl_data_source* source(const std::map<std::string, std::string>& data)
    {
        payloads = data;
        auto* source = wl_data_device_manager_create_data_source(manager);
        sources.push_back(source);
        static const wl_data_source_listener listener = {
            [](void*, wl_data_source*, const char*) {},
            [](void* data, wl_data_source*, const char* mime, int fd) {
                auto& c = *static_cast<Client*>(data);
                ++c.sends;
                fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
                c.writes.push_back({fd, c.payloads[mime]});
            },
            [](void* data, wl_data_source*) {
                ++static_cast<Client*>(data)->cancelled;
            },
            [](void* data, wl_data_source*) {
                ++static_cast<Client*>(data)->performed;
            },
            [](void* data, wl_data_source*) {
                ++static_cast<Client*>(data)->finished;
            },
            [](void* data, wl_data_source*, uint32_t action) {
                static_cast<Client*>(data)->sourceAction = action;
            }};
        wl_data_source_add_listener(source, &listener, this);
        for (const auto& [mime, bytes] : data)
            wl_data_source_offer(source, mime.c_str());
        return source;
    }

    void pump()
    {
        if (wl_display_get_error(display))
            return;
        const auto flushed = wl_display_flush(display);
        const auto dispatched = wl_display_dispatch_pending(display);
        if (!expectError) {
            EXPECT_GE(flushed, 0);
            EXPECT_GE(dispatched, 0);
        }
        pollfd pollFd {wl_display_get_fd(display), POLLIN, 0};
        if (poll(&pollFd, 1, 0) > 0 && (pollFd.revents & POLLIN))
            wl_display_dispatch(display);
        if (stall)
            return;
        for (auto it = writes.begin(); it != writes.end();) {
            const auto n =
                write(it->fd, it->bytes.data() + it->offset, it->bytes.size() - it->offset);
            if (n > 0)
                it->offset += n;
            if (it->offset == it->bytes.size() || (n < 0 && errno != EAGAIN && errno != EINTR)) {
                close(it->fd);
                it = writes.erase(it);
            } else {
                ++it;
            }
        }
    }

    ~Client()
    {
        for (auto& w : writes)
            close(w.fd);
        for (auto* offer : offers)
            wl_data_offer_destroy(offer);
        for (auto* source : sources)
            wl_data_source_destroy(source);
        for (auto* device : devices) {
            if (version >= 2)
                wl_data_device_release(device);
            else
                wl_data_device_destroy(device);
        }
        if (touch)
            wl_touch_release(touch);
        if (pointer)
            wl_pointer_release(pointer);
        if (keyboard)
            wl_keyboard_release(keyboard);
        if (surface)
            wl_surface_destroy(surface);
        if (manager)
            wl_data_device_manager_destroy(manager);
        if (seat)
            wl_seat_release(seat);
        if (compositor)
            wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_flush(display);
        wl_display_disconnect(display);
    }
};

class Harness
{
public:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    input::Seat seat {&compositor};
    wayland::Server server;
    wayland::WaylandSeat waylandSeat {&seat, server.serialTracker()};
    std::vector<std::unique_ptr<Client>> clients;

    struct sigaction previousSigpipe {};

    Harness()
    {
        struct sigaction action {};
        action.sa_handler = SIG_IGN;
        sigemptyset(&action.sa_mask);
        sigaction(SIGPIPE, &action, &previousSigpipe);
        wayland::createCompositorGlobal(server.display(), &compositor);
        wayland::createSeatGlobal(server.display(), &waylandSeat);
        wayland::createDataDeviceManagerGlobal(server.display(), &waylandSeat);
    }

    ~Harness()
    {
        clients.clear();
        wl_display_destroy_clients(server.display());
        sigaction(SIGPIPE, &previousSigpipe, nullptr);
    }

    void pump()
    {
        for (auto& c : clients)
            c->pump();
        EXPECT_TRUE(server.dispatch(std::chrono::milliseconds(1)));
        server.flush();
        test::processPendingEvents();
    }

    bool until(std::function<bool()> done, int timeout = 3000)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
        while (!done() && std::chrono::steady_clock::now() < deadline) {
            pump();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        return done();
    }

    void sync(Client& c)
    {
        bool done = false;
        auto* callback = wl_display_sync(c.display);
        static const wl_callback_listener listener = {
            [](void* data, wl_callback* callback, uint32_t) {
                *static_cast<bool*>(data) = true;
                wl_callback_destroy(callback);
            }};
        wl_callback_add_listener(callback, &listener, &done);
        if (!until([&] {
                return done;
            })) {
            wl_callback_destroy(callback);
            FAIL() << "client roundtrip timed out";
        }
    }

    Client& addClient(bool device = true, uint32_t version = 3,
                      const std::function<wl_client*(int)>& connect = {})
    {
        clients.push_back(std::make_unique<Client>(server, version, connect));
        auto& c = *clients.back();
        sync(c);
        c.initialize(device);
        sync(c);
        return c;
    }

    void focus(Client& c)
    {
        auto* resource = wl_client_get_object(
            c.serverClient, wl_proxy_get_id(reinterpret_cast<wl_proxy*>(c.surface)));
        seat.setFocusedSurface(wayland::getUserDataAs<compositor::Surface>(resource));
        sync(c);
    }

    void publish(Client& c, const std::map<std::string, std::string>& payloads)
    {
        focus(c);
        wl_data_device_set_selection(c.devices.front(), c.source(payloads), c.serial);
        sync(c);
    }

    std::string receive(Client& c, wl_data_offer* offer, const std::string& mime)
    {
        int fds[2];
        EXPECT_EQ(0, pipe2(fds, O_CLOEXEC | O_NONBLOCK));
        wl_data_offer_receive(offer, mime.c_str(), fds[1]);
        close(fds[1]);
        std::string bytes;
        bool eof = false;
        EXPECT_TRUE(until([&] {
            char buffer[4096];
            const auto n = read(fds[0], buffer, sizeof(buffer));
            if (n > 0)
                bytes.append(buffer, n);
            if (n == 0)
                eof = true;
            return eof;
        }));
        close(fds[0]);
        return bytes;
    }
};

} // namespace clipboard_test

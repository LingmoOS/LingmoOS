// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/geometry.h"
#include "input/event_sink.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/serial_tracker.h"
#include <atomic>
#include <chrono>
#include <climits>
#include <cstring>
#include <ext-idle-notify-v1-client-protocol.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <thread>
#include <wayland-client.h>
#include <wayland-server-core.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "scene/scene.h"
#include "wayland/idle_inhibition.h"
#include "wayland/protocols/idle_notify.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/zwp_idle_inhibit_manager.h"
#include <idle-inhibit-unstable-v1-client-protocol.h>

using namespace Lingmo;

namespace {

class IdleNotifyProtocolTest : public testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wayland::IdleInhibition inhibition;
    wayland::SerialTracker serials {nullptr};
    wayland::WaylandSeat waylandSeat {&seat, &serials};
    wl_display *server = nullptr;
    wl_display *client = nullptr;
    wl_registry *registry = nullptr;
    wl_seat *clientSeat = nullptr;
    ext_idle_notifier_v1 *notifier = nullptr;
    zwp_idle_inhibit_manager_v1 *inhibitManager = nullptr;
    wl_compositor *clientCompositor = nullptr;

    void SetUp() override
    {
        server = wl_display_create();
        ASSERT_TRUE(server);
        ASSERT_TRUE(wayland::createSeatGlobal(server, &waylandSeat));
        inhibition.input = &input;
        ASSERT_TRUE(wayland::createIdleNotifierGlobal(server, &inhibition));
        ASSERT_TRUE(wayland::createIdleInhibitManagerGlobal(server, &inhibition));
        ASSERT_TRUE(wayland::createCompositorGlobal(server, &compositor));
        int sockets[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets), 0);
        ASSERT_TRUE(wl_client_create(server, sockets[0])) << std::strerror(errno);
        client = wl_display_connect_to_fd(sockets[1]);
        ASSERT_TRUE(client);
        registry = wl_display_get_registry(client);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t) {
                    auto *self = static_cast<IdleNotifyProtocolTest *>(data);
                    if (std::strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
                        self->inhibitManager =
                                static_cast<zwp_idle_inhibit_manager_v1 *>(wl_registry_bind(
                                        registry, name, &zwp_idle_inhibit_manager_v1_interface, 1));
                    if (std::strcmp(interface, "wl_compositor") == 0)
                        self->clientCompositor = static_cast<wl_compositor *>(
                                wl_registry_bind(registry, name, &wl_compositor_interface, 1));
                    if (std::strcmp(interface, "wl_seat") == 0)
                        self->clientSeat = static_cast<wl_seat *>(
                                wl_registry_bind(registry, name, &wl_seat_interface, 1));
                    else if (std::strcmp(interface, "ext_idle_notifier_v1") == 0)
                        self->notifier = static_cast<ext_idle_notifier_v1 *>(wl_registry_bind(
                                registry, name, &ext_idle_notifier_v1_interface, 1));
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(registry, &listener, this);
        ASSERT_GE(roundtrip(), 0);
        ASSERT_TRUE(notifier);
        ASSERT_TRUE(clientSeat);
    }

    int roundtrip()
    {
        std::atomic<bool> done = false;
        int result = -1;
        std::thread worker([&] {
            result = wl_display_roundtrip(client);
            done = true;
        });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!done && std::chrono::steady_clock::now() < deadline) {
            wl_event_loop_dispatch(wl_display_get_event_loop(server), 1);
            wl_display_flush_clients(server);
        }
        if (!done)
            shutdown(wl_display_get_fd(client), SHUT_RDWR);
        worker.join();
        return result;
    }

    struct Events {
        int idled = 0;
        int resumed = 0;
    };

    ext_idle_notification_v1 *watch(uint32_t timeout, Events &events)
    {
        auto *notification =
                ext_idle_notifier_v1_get_idle_notification(notifier, timeout, clientSeat);
        static const ext_idle_notification_v1_listener listener = {
                [](void *data, ext_idle_notification_v1 *) {
                    ++static_cast<Events *>(data)->idled;
                },
                [](void *data, ext_idle_notification_v1 *) {
                    ++static_cast<Events *>(data)->resumed;
                }};
        ext_idle_notification_v1_add_listener(notification, &listener, &events);
        return notification;
    }

    void advance(int milliseconds)
    {
        const auto deadline =
                std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
        while (std::chrono::steady_clock::now() < deadline)
            wl_event_loop_dispatch(wl_display_get_event_loop(server), 1);
        ASSERT_GE(roundtrip(), 0);
    }

    void TearDown() override
    {
        if (inhibitManager)
            zwp_idle_inhibit_manager_v1_destroy(inhibitManager);
        if (clientCompositor)
            wl_compositor_destroy(clientCompositor);
        if (notifier)
            ext_idle_notifier_v1_destroy(notifier);
        if (clientSeat)
            wl_seat_destroy(clientSeat);
        if (registry)
            wl_registry_destroy(registry);
        if (client)
            wl_display_disconnect(client);
        if (server) {
            wl_display_destroy_clients(server);
            wl_display_destroy(server);
        }
    }
};

TEST_F(IdleNotifyProtocolTest, TimeoutsRestartOnConsumedInputAndDoNotRepeat)
{
    Events shortEvents, longEvents;
    auto *shortWatch = watch(30, shortEvents);
    auto *longWatch = watch(200, longEvents);
    ASSERT_GE(roundtrip(), 0);
    advance(55);
    EXPECT_EQ(shortEvents.idled, 1);
    EXPECT_EQ(shortEvents.resumed, 0);
    EXPECT_EQ(longEvents.idled, 0);
    advance(30);
    EXPECT_EQ(shortEvents.idled, 1);

    struct Sink : input::EventSink {
        bool onKey(input::Timestamp, uint32_t, bool) override { return true; }
    } sink;

    input.setEventSink(&sink);
    input.onKey(input::Timestamp {}, 30, true);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(shortEvents.resumed, 1);
    EXPECT_EQ(longEvents.resumed, 0);
    advance(55);
    EXPECT_EQ(shortEvents.idled, 2);
    EXPECT_EQ(longEvents.idled, 0);
    input.setEventSink(nullptr);
    ext_idle_notification_v1_destroy(shortWatch);
    ext_idle_notification_v1_destroy(longWatch);
}

TEST_F(IdleNotifyProtocolTest, ZeroAndLargeTimeoutsSurviveManagerAndSeatProxyDestruction)
{
    Events zeroEvents, largeEvents;
    auto *zero = watch(0, zeroEvents);
    auto *large = watch(UINT32_MAX, largeEvents);
    ext_idle_notifier_v1_destroy(notifier);
    notifier = nullptr;
    wl_seat_destroy(clientSeat);
    clientSeat = nullptr;
    ASSERT_GE(roundtrip(), 0);
    advance(10);
    EXPECT_EQ(zeroEvents.idled, 1);
    EXPECT_EQ(largeEvents.idled, 0);
    input.onPointerMotion(input::Timestamp {}, base::PointF(1, 1));
    advance(10);
    EXPECT_EQ(zeroEvents.resumed, 1);
    EXPECT_EQ(zeroEvents.idled, 2);
    EXPECT_EQ(largeEvents.resumed, 0);
    ext_idle_notification_v1_destroy(zero);
    ext_idle_notification_v1_destroy(large);
    ASSERT_GE(roundtrip(), 0);
    input.onPointerButton(input::Timestamp {}, 272, true);
    advance(10);
    EXPECT_EQ(zeroEvents.resumed, 1);
}

TEST_F(IdleNotifyProtocolTest, NewNotificationStartsItsOwnTimeout)
{
    advance(50);
    Events events;
    auto *notification = watch(100, events);
    ASSERT_GE(roundtrip(), 0);
    advance(20);
    EXPECT_EQ(events.idled, 0);
    advance(110);
    EXPECT_EQ(events.idled, 1);
    ext_idle_notification_v1_destroy(notification);
}

TEST_F(IdleNotifyProtocolTest, InhibitionResumesAndRestartsTimeoutAfterLastRelease)
{
    Events events;
    auto *notification = watch(30, events);
    ASSERT_GE(roundtrip(), 0);
    advance(55);
    ASSERT_EQ(events.idled, 1);
    inhibition.acquire();
    inhibition.acquire();
    Events inhibitedEvents;
    auto *inhibitedWatch = watch(30, inhibitedEvents);
    ASSERT_GE(roundtrip(), 0);
    advance(55);
    EXPECT_EQ(inhibitedEvents.idled, 0);
    EXPECT_EQ(events.resumed, 1);
    EXPECT_EQ(events.idled, 1);
    inhibition.release();
    advance(55);
    EXPECT_EQ(events.idled, 1);
    inhibition.release();
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(events.idled, 1);
    advance(55);
    EXPECT_EQ(events.idled, 2);
    EXPECT_EQ(inhibitedEvents.idled, 1);
    ext_idle_notification_v1_destroy(notification);
    ext_idle_notification_v1_destroy(inhibitedWatch);
}

TEST_F(IdleNotifyProtocolTest, IgnoredInhibitionLetsTimeoutsRunUntilItCountsAgain)
{
    Events events;
    auto *notification = watch(30, events);
    inhibition.acquire();
    ASSERT_GE(roundtrip(), 0);
    advance(55);
    EXPECT_EQ(events.idled, 0);

    inhibition.setIgnored(true);
    EXPECT_FALSE(inhibition.inhibited());
    advance(55);
    EXPECT_EQ(events.idled, 1);

    // Held all along, so it counts again as soon as it is no longer ignored.
    inhibition.setIgnored(false);
    EXPECT_TRUE(inhibition.inhibited());
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(events.resumed, 1);
    advance(55);
    EXPECT_EQ(events.idled, 1);

    inhibition.release();
    ext_idle_notification_v1_destroy(notification);
}

TEST_F(IdleNotifyProtocolTest, SurfaceVisibilityAndResourceLifetimes)
{
    class Buffer : public compositor::Buffer
    {
    public:
        Lingmo::base::Size size() const override { return {10, 10}; }

        render::PixelFormat format() const override { return render::PixelFormat::Bgra8888; }

        const uint8_t *pixels() const override { return nullptr; }

        int stride() const override { return 40; }
    };

    auto *surface = wl_compositor_create_surface(clientCompositor);
    auto *first = zwp_idle_inhibit_manager_v1_create_inhibitor(inhibitManager, surface);
    auto *second = zwp_idle_inhibit_manager_v1_create_inhibitor(inhibitManager, surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(inhibition.inhibited());
    ASSERT_EQ(compositor.surfaces().size(), 1u);
    auto *model = compositor.surfaces().front();
    auto buffer = std::make_shared<Buffer>();
    model->attach(buffer, {});
    model->commit();
    EXPECT_TRUE(inhibition.inhibited());
    model->setHidden(true);
    inhibition.sceneChanged();
    EXPECT_FALSE(inhibition.inhibited());
    model->setHidden(false);
    inhibition.sceneChanged();
    EXPECT_TRUE(inhibition.inhibited());
    model->attach(nullptr, {});
    model->commit();
    EXPECT_FALSE(inhibition.inhibited());
    model->attach(buffer, {});
    model->commit();
    EXPECT_TRUE(inhibition.inhibited());
    zwp_idle_inhibit_manager_v1_destroy(inhibitManager);
    inhibitManager = nullptr;
    zwp_idle_inhibitor_v1_destroy(first);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(inhibition.inhibited());
    wl_surface_destroy(surface);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_FALSE(inhibition.inhibited());
    zwp_idle_inhibitor_v1_destroy(second);
    ASSERT_GE(roundtrip(), 0);
}

} // namespace

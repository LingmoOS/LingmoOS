// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A Lingmo stand-in for chromium/src/components/exo/wayland/test/
// wayland_server_test_base.*: a real libwayland client on its own thread,
// driven by a Server running on the event loop of the main thread.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "loop_test_util.h"
#include "temporary_directory.h"
#include "wayland/server.h"
#include "wayland/server_util.h"

#include <gtest/gtest.h>
#include <wayland-client.h>
#include <wayland-server-protocol.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

namespace Lingmo::Wayland {
namespace {

// Runs a wl_display connection on its own thread so that the server can keep
// dispatching on the main thread's event loop.
class TestClient
{
public:
    void start(const std::string& socketName)
    {
        m_thread = std::thread([this, name = socketName] { run(name); });
    }

    void requestDisconnect() { m_shouldDisconnect = true; }

    void join()
    {
        m_shouldDisconnect = true;
        if (m_thread.joinable())
            m_thread.join();
    }

    bool connected() const { return m_connected; }
    bool roundtripDone() const { return m_roundtripDone; }
    bool finished() const { return m_finished; }
    int globalCount() const { return m_globalCount; }

private:
    void run(const std::string& socketName)
    {
        wl_display* display = wl_display_connect(socketName.c_str());
        if (!display) {
            m_finished = true;
            return;
        }
        m_connected = true;

        static const wl_registry_listener listener = {
            [](void* data, wl_registry*, uint32_t, const char*, uint32_t) {
                ++*static_cast<std::atomic<int>*>(data);
            },
            [](void*, wl_registry*, uint32_t) {},
        };
        wl_registry* registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &listener, &m_globalCount);

        // Blocks until the server answers, which only happens if the loop
        // integration is dispatching and flushing.
        const bool ok = wl_display_roundtrip(display) >= 0;
        m_roundtripDone = ok;

        while (!m_shouldDisconnect)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        m_finished = true;
    }

    std::thread m_thread;
    std::atomic<bool> m_connected { false };
    std::atomic<bool> m_roundtripDone { false };
    std::atomic<bool> m_shouldDisconnect { false };
    std::atomic<bool> m_finished { false };
    std::atomic<int> m_globalCount { 0 };
};

class WaylandServerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_runtimeDir.isValid());
        setenv("XDG_RUNTIME_DIR", m_runtimeDir.path().c_str(), 1);

        m_server = std::make_unique<Server>();
        ASSERT_TRUE(m_server->display());
    }

    void TearDown() override { m_server.reset(); }

    // Spins the event loop, which is what drives the server, until
    // `predicate` holds.
    template<class Predicate>
    bool waitFor(Predicate predicate, int timeoutMs = 5000)
    {
        return test::runUntil(predicate, std::chrono::milliseconds(timeoutMs));
    }

    test::TemporaryDirectory m_runtimeDir;
    std::unique_ptr<Server> m_server;
};

TEST_F(WaylandServerTest, AddSocketAutoPicksAName)
{
    EXPECT_TRUE(m_server->socketName().empty());
    ASSERT_TRUE(m_server->addSocket());
    EXPECT_TRUE(m_server->socketName().starts_with("wayland-"));
}

TEST_F(WaylandServerTest, AddSocketHonoursAnExplicitName)
{
    ASSERT_TRUE(m_server->addSocket("lingmo-test-0"));
    EXPECT_EQ("lingmo-test-0", m_server->socketName());
}

TEST_F(WaylandServerTest, EventLoopFdIsExposedOnlyWithADisplay)
{
    EXPECT_GE(m_server->fileDescriptor(), 0);
    EXPECT_TRUE(m_server->dispatch(std::chrono::milliseconds(0)));
}

TEST_F(WaylandServerTest, ServerForDisplayResolvesBackToTheServer)
{
    EXPECT_EQ(m_server.get(), Server::serverForDisplay(m_server->display()));
    EXPECT_EQ(nullptr, Server::serverForDisplay(nullptr));
}

TEST_F(WaylandServerTest, ClientConnectsAndSeesAnEmptyRegistry)
{
    ASSERT_TRUE(m_server->addSocket());
    ASSERT_TRUE(m_server->start());

    TestClient client;
    client.start(m_server->socketName());

    ASSERT_TRUE(waitFor([&] { return client.roundtripDone(); }));
    EXPECT_TRUE(client.connected());
    EXPECT_EQ(1, m_server->trackedClientCount());
    // A bare server registers no globals.
    EXPECT_EQ(0, client.globalCount());

    client.requestDisconnect();
    ASSERT_TRUE(waitFor([&] { return client.finished(); }));

    // The server notices the disconnect once it dispatches again.
    ASSERT_TRUE(waitFor([&] { return m_server->trackedClientCount() == 0; }));
    client.join();
}

TEST_F(WaylandServerTest, ServerShutsDownWithAClientStillConnected)
{
    ASSERT_TRUE(m_server->addSocket());
    ASSERT_TRUE(m_server->start());

    TestClient client;
    client.start(m_server->socketName());
    ASSERT_TRUE(waitFor([&] { return client.roundtripDone(); }));
    EXPECT_EQ(1, m_server->trackedClientCount());

    // Destroying the server tears the client down from the server side.
    m_server.reset();

    client.requestDisconnect();
    ASSERT_TRUE(waitFor([&] { return client.finished(); }));
    client.join();
}

TEST_F(WaylandServerTest, DispatchWithoutTheEventLoopAcceptsAClient)
{
    ASSERT_TRUE(m_server->addSocket());

    struct Listener {
        wl_listener listener;
        bool notified = false;
    } created;
    created.listener.notify = [](wl_listener* listener, void*) {
        reinterpret_cast<Listener*>(listener)->notified = true;
    };
    wl_display_add_client_created_listener(m_server->display(), &created.listener);

    TestClient client;
    client.start(m_server->socketName());
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    const auto hasExpired = [&] { return std::chrono::steady_clock::now() >= deadline; };
    while (!created.notified && !hasExpired())
        ASSERT_TRUE(m_server->dispatch(std::chrono::milliseconds(10)));
    wl_list_remove(&created.listener.link);
    ASSERT_TRUE(created.notified);

    // The roundtrip needs the server to dispatch and flush by hand too.
    while (!client.roundtripDone() && !hasExpired()) {
        m_server->dispatch(std::chrono::milliseconds(10));
        m_server->flush();
    }
    EXPECT_TRUE(client.roundtripDone());

    client.requestDisconnect();
    while (!client.finished() && !hasExpired())
        m_server->dispatch(std::chrono::milliseconds(10));
    while (m_server->trackedClientCount() != 0 && !hasExpired())
        m_server->dispatch(std::chrono::milliseconds(10));
    EXPECT_EQ(0, m_server->trackedClientCount());
    client.join();
}

TEST_F(WaylandServerTest, FlushWithoutClients)
{
    m_server->flush();
    ASSERT_TRUE(m_server->addSocket());
    ASSERT_TRUE(m_server->start());
    m_server->flush();
}

// Regression test for crbug.com/1508130: a resource destructor that flushes
// while its client is being destroyed, with events still queued for it.
TEST_F(WaylandServerTest, FlushDoesNotCrashDuringClientDisconnect)
{
    ASSERT_TRUE(m_server->addSocket());
    ASSERT_TRUE(m_server->start());

    TestClient client;
    client.start(m_server->socketName());
    ASSERT_TRUE(waitFor([&] { return client.roundtripDone(); }));

    wl_list* clients = wl_display_get_client_list(m_server->display());
    ASSERT_FALSE(wl_list_empty(clients));
    wl_client* wlClient = wl_client_from_link(clients->next);
    EXPECT_FALSE(m_server->isClientDestroyed(wlClient));

    struct Context {
        Server* server;
        wl_client* client;
        bool destroyed = false;
    } context {m_server.get(), wlClient};
    wl_resource* output = wl_resource_create(wlClient, &wl_output_interface, 4, 0);
    ASSERT_NE(nullptr, output);
    wl_resource_set_implementation(output, nullptr, &context, [](wl_resource* resource) {
        auto* context = static_cast<Context*>(wl_resource_get_user_data(resource));
        EXPECT_TRUE(context->server->isClientDestroyed(context->client));
        context->server->flush();
        context->destroyed = true;
    });
    wl_output_send_name(output, "test_output");

    client.requestDisconnect();
    ASSERT_TRUE(waitFor([&] { return context.destroyed; }));
    ASSERT_TRUE(waitFor([&] { return m_server->trackedClientCount() == 0; }));
    client.join();
}

} // namespace
} // namespace Lingmo::Wayland

// Found by protocol_fuzzer: a region built from INT_MAX-sized arguments
// overflowed the rectangle's right edge.
TEST(ClientRect, EdgesStayWithinRangeAndNegativeSizesAreEmpty)
{
    using Lingmo::base::Rect;
    using Lingmo::wayland::clientRect;
    EXPECT_EQ(clientRect(10, 20, 30, 40), Rect(10, 20, 30, 40));
    EXPECT_EQ(clientRect(-5, -6, 10, 12), Rect(-5, -6, 10, 12));
    EXPECT_TRUE(clientRect(10, 20, -30, 40).isEmpty());
    EXPECT_TRUE(clientRect(10, 20, 30, 0).isEmpty());

    const int limit = 1 << 24;
    EXPECT_EQ(clientRect(1828966744, 0, 1353190006, 1), Rect(limit, 0, 0, 1));
    EXPECT_EQ(clientRect(0, 0, INT32_MAX, INT32_MAX), Rect(0, 0, limit, limit));
    EXPECT_EQ(clientRect(INT32_MIN, INT32_MIN, INT32_MAX, 5),
              Rect(-limit, -limit, limit - 1, 0));
    EXPECT_EQ(clientRect(-limit - 10, 0, 20, 1), Rect(-limit, 0, 10, 1));
}

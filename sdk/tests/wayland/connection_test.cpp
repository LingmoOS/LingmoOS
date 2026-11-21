// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <poll.h>
#include <sys/socket.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include <wayland-client.h>
#include <wayland-server.h>

#include "fractional-scale-v1-server-protocol.h"
#include "viewporter-server-protocol.h"
#include "xdg-shell-server-protocol.h"

#include "base/event_loop.h"
#include "wayland/connection.h"

using namespace Lingmo;
using namespace Lingmo::UI;

namespace {

// The globals a Connection needs, on a thread of its own; destroying it
// closes the connection as a compositor that exits or restarts does.
class FakeCompositor
{
public:
    FakeCompositor()
    {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds);
        clientFd = fds[1];
        m_display = wl_display_create();
        global(&wl_compositor_interface, 4);
        global(&xdg_wm_base_interface, 1);
        global(&wp_viewporter_interface, 1);
        global(&wp_fractional_scale_manager_v1_interface, 1);
        wl_client_create(m_display, fds[0]);
        m_thread = std::thread([this] {
            wl_event_loop* loop = wl_display_get_event_loop(m_display);
            while (!m_stop) {
                wl_event_loop_dispatch(loop, 10);
                wl_display_flush_clients(m_display);
            }
        });
    }

    ~FakeCompositor()
    {
        m_stop = true;
        m_thread.join();
        wl_display_destroy_clients(m_display);
        wl_display_destroy(m_display);
    }

    int clientFd = -1;

private:
    void global(const wl_interface* interface, int version)
    {
        wl_global_create(m_display, interface, version, const_cast<wl_interface*>(interface), bindIgnored);
    }

    static int ignore(const void*, void*, uint32_t, const wl_message*, wl_argument*) { return 0; }

    static void bindIgnored(wl_client* client, void* data, uint32_t version, uint32_t id)
    {
        wl_resource* resource = wl_resource_create(client, static_cast<const wl_interface*>(data), int(version), id);
        wl_resource_set_dispatcher(resource, ignore, nullptr, nullptr, nullptr);
    }

    wl_display* m_display = nullptr;
    std::thread m_thread;
    std::atomic<bool> m_stop{false};
};

} // namespace

// A loop run inside another one only dispatches: the lost connection asks
// it to end once, through the quit handler, and leaves nothing readable
// behind for the outer loop to wake up for again.
TEST(ConnectionTest, LostConnectionEndsAnEmbeddedLoopOnce)
{
    auto compositor = std::make_unique<FakeCompositor>();
    base::EventLoop loop;
    std::vector<int> quits;
    loop.setQuitHandler([&](int exitCode) { quits.push_back(exitCode); });
    std::unique_ptr<wayland::Connection> connection = wayland::Connection::openFd(loop, compositor->clientFd);
    ASSERT_TRUE(connection);

    compositor.reset();
    pollfd ready{loop.fd(), POLLIN, 0};
    for (int i = 0; i < 50 && quits.empty(); ++i) {
        poll(&ready, 1, 100);
        loop.dispatch();
    }
    EXPECT_EQ(quits, std::vector<int>{1});
    EXPECT_TRUE(connection->isLost());

    for (int i = 0; i < 10; ++i)
        loop.dispatch();
    EXPECT_EQ(quits.size(), 1u);
    EXPECT_EQ(poll(&ready, 1, 0), 0);
}

// A loop of its own returns with a failure, as the program then exits.
TEST(ConnectionTest, LostConnectionEndsRun)
{
    auto compositor = std::make_unique<FakeCompositor>();
    base::EventLoop loop;
    std::unique_ptr<wayland::Connection> connection = wayland::Connection::openFd(loop, compositor->clientFd);
    ASSERT_TRUE(connection);

    compositor.reset();
    EXPECT_EQ(loop.run(), 1);
}

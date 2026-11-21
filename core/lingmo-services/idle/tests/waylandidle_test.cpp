#include "waylandidle.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <sys/epoll.h>
#include <vector>
#include <wayland-server.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/timer.h"

#include "ext-idle-notify-v1-server-protocol.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;

namespace {

// Runs the loop until the condition holds or the time is up.
bool waitFor(EventLoop &loop, const std::function<bool()> &condition, std::chrono::milliseconds timeout = 5s)
{
    if (condition())
        return true;
    Timer poll(&loop);
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    Timer limit(&loop);
    limit.setSingleShot(true);
    limit.onTimeout = [&loop] {
        loop.quit();
    };
    poll.start(5ms);
    limit.start(timeout);
    loop.run();
    return condition();
}

// A compositor with a seat and the idle notifier, dispatched by the test's loop.
class WaylandIdleTest : public testing::Test
{
protected:
    struct Notification {
        wl_resource *resource;
        uint32_t timeout;
    };

    EventLoop loop;
    file::TemporaryDirectory runtime {"/tmp", "idle-test"};
    std::vector<std::unique_ptr<Notification>> notifications;
    wl_display *server = nullptr;
    int watch = 0;

    void SetUp() override
    {
        ASSERT_TRUE(runtime.isValid());
        setenv("XDG_RUNTIME_DIR", runtime.path().c_str(), 1);
        setenv("WAYLAND_DISPLAY", "idle-test", 1);
        startServer();
    }

    void TearDown() override
    {
        if (server)
            stopServer();
    }

    void startServer()
    {
        server = wl_display_create();
        ASSERT_TRUE(server);
        ASSERT_EQ(wl_display_add_socket(server, "idle-test"), 0);
        wl_global_create(server, &wl_seat_interface, 1, nullptr, [](wl_client *client, void *, uint32_t, uint32_t id) {
            auto *resource = wl_resource_create(client, &wl_seat_interface, 1, id);
            wl_resource_set_implementation(resource, nullptr, nullptr, nullptr);
        });
        wl_global_create(
                server, &ext_idle_notifier_v1_interface, 1, this, [](wl_client *client, void *data, uint32_t, uint32_t id) {
                    auto *resource = wl_resource_create(client, &ext_idle_notifier_v1_interface, 1, id);
                    static const struct ext_idle_notifier_v1_interface implementation = {
                            [](wl_client *, wl_resource *resource) {
                                wl_resource_destroy(resource);
                            },
                            [](wl_client *client, wl_resource *manager, uint32_t id, uint32_t timeout, wl_resource *) {
                                auto *self = static_cast<WaylandIdleTest *>(wl_resource_get_user_data(manager));
                                auto *resource =
                                        wl_resource_create(client, &ext_idle_notification_v1_interface, 1, id);
                                self->notifications.push_back(std::make_unique<Notification>(Notification {resource, timeout}));
                                static const struct ext_idle_notification_v1_interface implementation = {
                                        [](wl_client *, wl_resource *resource) {
                                            wl_resource_destroy(resource);
                                        }};
                                wl_resource_set_implementation(resource, &implementation, self, [](wl_resource *resource) {
                                    auto *self = static_cast<WaylandIdleTest *>(wl_resource_get_user_data(resource));
                                    std::erase_if(self->notifications, [resource](const auto &notification) {
                                        return notification->resource == resource;
                                    });
                                });
                            }};
                    wl_resource_set_implementation(resource, &implementation, data, nullptr);
                });
        wl_event_loop *events = wl_display_get_event_loop(server);
        watch = loop.watch(wl_event_loop_get_fd(events), EPOLLIN, [this, events](uint32_t) {
            wl_event_loop_dispatch(events, 0);
            wl_display_flush_clients(server);
        });
    }

    void stopServer()
    {
        loop.unwatch(watch);
        watch = 0;
        wl_display_destroy_clients(server);
        wl_display_destroy(server);
        server = nullptr;
    }
};

} // namespace

TEST_F(WaylandIdleTest, DispatchRemoveAndReconnect)
{
    WaylandIdle idle;
    std::vector<std::pair<int, int>> timeouts;
    int resumes = 0;
    auto timeoutSubscription = idle.timeoutReached.connect([&](int id, int milliseconds) {
        timeouts.emplace_back(id, milliseconds);
    });
    auto resumeSubscription = idle.resumed.connect([&] {
        ++resumes;
    });

    const int id = idle.addTimeout(250);
    ASSERT_TRUE(waitFor(loop, [&] { return notifications.size() == 1; }));
    EXPECT_EQ(notifications.front()->timeout, 250u);
    ext_idle_notification_v1_send_idled(notifications.front()->resource);
    wl_display_flush_clients(server);
    ASSERT_TRUE(waitFor(loop, [&] { return timeouts.size() == 1; }));
    EXPECT_EQ(timeouts.front(), std::make_pair(id, 250));
    ext_idle_notification_v1_send_resumed(notifications.front()->resource);
    wl_display_flush_clients(server);
    ASSERT_TRUE(waitFor(loop, [&] { return resumes == 1; }));
    idle.removeTimeout(id);
    ASSERT_TRUE(waitFor(loop, [&] { return notifications.empty(); }));
    const int retained = idle.addTimeout(700);
    ASSERT_TRUE(waitFor(loop, [&] { return notifications.size() == 1; }));
    stopServer();
    ASSERT_TRUE(waitFor(loop, [&] { return resumes == 2; }));
    startServer();
    ASSERT_TRUE(waitFor(loop, [&] { return notifications.size() == 1; }, 3s));
    EXPECT_EQ(notifications.front()->timeout, 700u);
    ext_idle_notification_v1_send_idled(notifications.front()->resource);
    wl_display_flush_clients(server);
    ASSERT_TRUE(waitFor(loop, [&] { return timeouts.size() == 2; }));
    EXPECT_EQ(timeouts.back(), std::make_pair(retained, 700));
}

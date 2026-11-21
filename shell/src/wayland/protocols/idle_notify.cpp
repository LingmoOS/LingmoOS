// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/idle_notify.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <wayland-server-core.h>

#include "ext-idle-notify-v1-server-protocol.h"
#include "input/input_manager.h"
#include "wayland/idle_inhibition.h"
#include "wayland/protocols/wl_seat.h"
#include "base/signal.h"

namespace Lingmo::Wayland {
namespace {

struct Notification {
    wl_resource *resource;
    wl_event_source *timer = nullptr;
    base::Subscription activity;
    base::Subscription inhibitionChanged;
    IdleInhibition *inhibition;
    uint32_t timeout;
    bool idle = false;
    std::chrono::steady_clock::time_point deadline;

    ~Notification()
    {
        activity.reset();
        inhibitionChanged.reset();
        if (timer)
            wl_event_source_remove(timer);
    }

    void arm()
    {
        const auto remaining = std::chrono::ceil<std::chrono::milliseconds>(
                                       deadline - std::chrono::steady_clock::now())
                                       .count();
        // Wayland uses signed milliseconds; zero disarms the timer.
        wl_event_source_timer_update(timer, int(std::clamp<int64_t>(remaining, 1, INT_MAX)));
    }

    void reset()
    {
        if (idle) {
            idle = false;
            ext_idle_notification_v1_send_resumed(resource);
        }
        if (inhibition->inhibited()) {
            wl_event_source_timer_update(timer, 0);
            return;
        }
        deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
        arm();
    }
};

void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct ext_idle_notification_v1_interface notificationImplementation = {destroy};

void getNotification(wl_client *client, wl_resource *manager, uint32_t id, uint32_t timeout,
                     wl_resource *seat)
{
    auto *resource = wl_resource_create(client, &ext_idle_notification_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto *inhibition = static_cast<IdleInhibition *>(wl_resource_get_user_data(manager));
    auto *notification = new Notification {resource, nullptr, {}, {}, inhibition, timeout};
    wl_resource_set_implementation(
            resource, &notificationImplementation, notification, [](wl_resource *resource) {
                delete static_cast<Notification *>(wl_resource_get_user_data(resource));
            });
    if (!waylandSeatFromResource(seat))
        return;
    notification->timer = wl_event_loop_add_timer(
            wl_display_get_event_loop(wl_client_get_display(client)),
            [](void *data) {
                auto *notification = static_cast<Notification *>(data);
                if (std::chrono::steady_clock::now() < notification->deadline) {
                    notification->arm();
                    return 0;
                }
                notification->idle = true;
                ext_idle_notification_v1_send_idled(notification->resource);
                return 0;
            },
            notification);
    if (!notification->timer) {
        wl_client_post_no_memory(client);
        return;
    }
    auto *input = inhibition->input;
    notification->inhibitionChanged = inhibition->inhibitedChanged.connect(
            [notification] {
                notification->reset();
            });
    notification->activity = input->userActivity.connect([notification] {
        notification->reset();
    });
    notification->reset();
}

const struct ext_idle_notifier_v1_interface managerImplementation = {destroy, getNotification};

void bind(wl_client *client, void *data, uint32_t, uint32_t id)
{
    auto *resource = wl_resource_create(client, &ext_idle_notifier_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, data, nullptr);
}

} // namespace

wl_global *createIdleNotifierGlobal(wl_display *display, IdleInhibition *inhibition)
{
    return wl_global_create(display, &ext_idle_notifier_v1_interface, 1, inhibition, bind);
}
} // namespace Lingmo::Wayland

#include "waylandidle.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <map>
#include <sys/epoll.h>
#include <wayland-client.h>

#include "base/event_loop.h"
#include "base/timer.h"

#include "ext-idle-notify-v1-client-protocol.h"

using namespace std::chrono_literals;
using namespace Lingmo::Base;

class WaylandIdle::Private
{
public:
    struct Timeout {
        Private *owner;
        int id;
        int milliseconds;
        ext_idle_notification_v1 *proxy = nullptr;
    };

    explicit Private(WaylandIdle *owner)
        : q(owner)
        , loop(*EventLoop::current())
    {
        retry.setSingleShot(true);
        retry.setInterval(1000ms);
        retry.onTimeout = [this] {
            start();
        };
        loop.post([this, alive = std::weak_ptr<bool>(alive)] {
            if (alive.lock())
                start();
        });
    }

    ~Private()
    {
        reset();
    }

    void reset()
    {
        if (watch)
            loop.unwatch(watch);
        watch = 0;
        for (auto &[id, timeout] : timeouts) {
            if (timeout->proxy)
                ext_idle_notification_v1_destroy(timeout->proxy);
            timeout->proxy = nullptr;
        }
        if (manager)
            ext_idle_notifier_v1_destroy(manager);
        if (seat)
            wl_seat_destroy(seat);
        if (registry)
            wl_registry_destroy(registry);
        if (display)
            wl_display_disconnect(display);
        manager = nullptr;
        seat = nullptr;
        registry = nullptr;
        display = nullptr;
        seatName = managerName = 0;
    }

    void failed()
    {
        if (failing)
            return;
        failing = true;
        if (watch)
            loop.setWatchEvents(watch, 0);
        // Protocol callbacks must finish before their listener data is freed.
        loop.post([this, alive = std::weak_ptr<bool>(alive)] {
            if (!alive.lock())
                return;
            reset();
            failing = false;
            q->resumed.emit();
            retry.start();
        });
    }

    void flush()
    {
        if (!display || failing)
            return;
        if (wl_display_flush(display) < 0) {
            if (errno == EAGAIN)
                loop.setWatchEvents(watch, EPOLLIN | EPOLLOUT);
            else
                failed();
        } else {
            loop.setWatchEvents(watch, EPOLLIN);
        }
    }

    void bindTimeout(Timeout *timeout)
    {
        if (!manager || !seat || timeout->proxy || failing)
            return;
        timeout->proxy = ext_idle_notifier_v1_get_idle_notification(
                manager, uint32_t(timeout->milliseconds), seat);
        static const ext_idle_notification_v1_listener listener = {
                [](void *data, ext_idle_notification_v1 *) {
                    auto *timeout = static_cast<Timeout *>(data);
                    timeout->owner->q->timeoutReached.emit(timeout->id, timeout->milliseconds);
                },
                [](void *data, ext_idle_notification_v1 *) {
                    auto *timeout = static_cast<Timeout *>(data);
                    timeout->owner->q->resumed.emit();
                }};
        ext_idle_notification_v1_add_listener(timeout->proxy, &listener, timeout);
    }

    void start()
    {
        // WAYLAND_DISPLAY, else wayland-0.
        display = wl_display_connect(nullptr);
        if (!display) {
            retry.start();
            return;
        }
        watch = loop.watch(wl_display_get_fd(display), EPOLLIN, [this](uint32_t events) {
            if (events & EPOLLOUT) {
                flush();
                if (!(events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
                    return;
            }
            if (wl_display_dispatch(display) < 0)
                failed();
            else
                flush();
        });
        registry = wl_display_get_registry(display);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface,
                   uint32_t) {
                    auto *self = static_cast<Private *>(data);
                    if (std::strcmp(interface, "wl_seat") == 0 && !self->seat) {
                        self->seatName = name;
                        self->seat = static_cast<wl_seat *>(
                                wl_registry_bind(registry, name, &wl_seat_interface, 1));
                    } else if (std::strcmp(interface, "ext_idle_notifier_v1") == 0 && !self->manager) {
                        self->managerName = name;
                        self->manager = static_cast<ext_idle_notifier_v1 *>(wl_registry_bind(
                                registry, name, &ext_idle_notifier_v1_interface, 1));
                    }
                    for (auto &[id, timeout] : self->timeouts)
                        self->bindTimeout(timeout.get());
                },
                [](void *data, wl_registry *, uint32_t name) {
                    auto *self = static_cast<Private *>(data);
                    if (name == self->seatName || name == self->managerName)
                        self->failed();
                }};
        wl_registry_add_listener(registry, &listener, this);
        flush();
    }

    WaylandIdle *q;
    EventLoop &loop;
    Timer retry;
    std::map<int, std::unique_ptr<Timeout>> timeouts;
    int nextId = 0;
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_seat *seat = nullptr;
    ext_idle_notifier_v1 *manager = nullptr;
    uint32_t seatName = 0;
    uint32_t managerName = 0;
    int watch = 0;
    bool failing = false;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
};

WaylandIdle::WaylandIdle()
    : d(std::make_unique<Private>(this))
{
}

WaylandIdle::~WaylandIdle() = default;

int WaylandIdle::addTimeout(int milliseconds)
{
    const int id = ++d->nextId;
    auto timeout = std::make_unique<Private::Timeout>(Private::Timeout {d.get(), id, std::max(0, milliseconds)});
    Private::Timeout *entry = timeout.get();
    d->timeouts.emplace(id, std::move(timeout));
    d->bindTimeout(entry);
    d->flush();
    return id;
}

void WaylandIdle::removeTimeout(int id)
{
    auto it = d->timeouts.find(id);
    if (it == d->timeouts.end())
        return;
    if (it->second->proxy)
        ext_idle_notification_v1_destroy(it->second->proxy);
    d->timeouts.erase(it);
    d->flush();
}

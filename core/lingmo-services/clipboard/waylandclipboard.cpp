#include "waylandclipboard.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/timer.h"

#include "ext-data-control-v1-client-protocol.h"

using namespace std::chrono_literals;
using namespace Lingmo::Base;

static constexpr size_t kMaxClipboardBytes = 64 * 1024 * 1024;
static constexpr size_t kMaxFormats = 128;
static constexpr auto kTransferTimeout = 5000ms;

namespace {

// Reads a pipe to its end, or writes bytes into one, without blocking.
class PipeTransfer
{
public:
    using Completion = std::function<void(bool, std::string)>;

    PipeTransfer(int fd, bool writing, std::string bytes, size_t limit, Completion completion)
        : m_loop(*EventLoop::current())
        , m_fd(fd)
        , m_writing(writing)
        , m_bytes(std::move(bytes))
        , m_limit(limit)
        , m_completion(std::move(completion))
    {
        const int flags = fcntl(fd, F_GETFL);
        struct stat status {};
        m_valid = flags >= 0 && fstat(fd, &status) == 0
                && (S_ISFIFO(status.st_mode) || S_ISSOCK(status.st_mode))
                && fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
        if (m_valid)
            m_watch = m_loop.watch(fd, writing ? EPOLLOUT : EPOLLIN, [this](uint32_t) {
                pump();
            });
        m_timeout.setSingleShot(true);
        m_timeout.onTimeout = [this] {
            finish(false);
        };
        m_timeout.start(kTransferTimeout);
        m_loop.post([this, alive = std::weak_ptr<bool>(m_alive)] {
            if (alive.lock())
                pump();
        });
    }

    ~PipeTransfer()
    {
        if (m_watch)
            m_loop.unwatch(m_watch);
        if (m_fd >= 0)
            close(m_fd);
    }

    PipeTransfer(const PipeTransfer &) = delete;
    PipeTransfer &operator=(const PipeTransfer &) = delete;

    bool isFinished() const { return m_fd < 0; }

private:
    void pump()
    {
        if (m_fd < 0)
            return;
        if (!m_valid) {
            finish(false);
            return;
        }
        for (int i = 0; i < 16; ++i) {
            char buffer[4096];
            if (m_writing && m_offset == m_bytes.size()) {
                finish(true);
                return;
            }
            const ssize_t n = m_writing
                    ? file::writeWithoutSigpipe(m_fd, m_bytes.data() + m_offset,
                                                std::min<size_t>(4096, m_bytes.size() - m_offset))
                    : read(m_fd, buffer, sizeof(buffer));
            if (n > 0) {
                if (m_writing) {
                    m_offset += size_t(n);
                } else {
                    if (size_t(n) > m_limit - m_bytes.size()) {
                        finish(false);
                        return;
                    }
                    m_bytes.append(buffer, size_t(n));
                }
            } else if (!m_writing && n == 0) {
                finish(true);
                return;
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return;
            } else if (n < 0 && errno == EINTR) {
                continue;
            } else {
                finish(false);
                return;
            }
        }
    }

    void finish(bool success)
    {
        if (m_fd < 0)
            return;
        m_timeout.stop();
        if (m_watch)
            m_loop.unwatch(m_watch);
        m_watch = 0;
        close(m_fd);
        m_fd = -1;
        auto completion = std::move(m_completion);
        auto bytes = std::move(m_bytes);
        if (completion)
            completion(success, std::move(bytes));
    }

    EventLoop &m_loop;
    int m_fd;
    int m_watch = 0;
    bool m_writing;
    bool m_valid = false;
    std::string m_bytes;
    size_t m_limit;
    size_t m_offset = 0;
    Completion m_completion;
    Timer m_timeout;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace

static void destroyProxy(wl_proxy *&proxy, uint32_t opcode)
{
    if (!proxy)
        return;
    wl_proxy_marshal_flags(proxy, opcode, nullptr, wl_proxy_get_version(proxy), WL_MARSHAL_FLAG_DESTROY);
    proxy = nullptr;
}

class WaylandClipboard::Private
{
public:
    explicit Private(WaylandClipboard *owner)
        : q(owner)
        , loop(*EventLoop::current())
    {
        loop.post([this, alive = std::weak_ptr<bool>(alive)] {
            if (alive.lock())
                start();
        });
    }

    ~Private()
    {
        *alive = false;
        reset();
    }

    struct Offer {
        wl_proxy *proxy = nullptr;
        std::vector<std::string> formats;
        bool tooManyFormats = false;

        ~Offer() { destroyProxy(proxy, 1); }
    };

    struct Source {
        wl_proxy *proxy = nullptr;
        std::map<std::string, std::string> bytes;
        std::vector<std::unique_ptr<PipeTransfer>> writes;

        ~Source() { destroyProxy(proxy, 1); }
    };

    void reset()
    {
        ++generation;
        reader.reset();
        selection.reset();
        pending.clear();
        source.reset();
        snapshot.reset();
        collecting.reset();
        destroyProxy(device, 1);
        destroyProxy(manager, 2);
        if (seat) {
            wl_seat_destroy(seat);
            seat = nullptr;
        }
        if (sync) {
            wl_callback_destroy(sync);
            sync = nullptr;
        }
        if (registry) {
            wl_registry_destroy(registry);
            registry = nullptr;
        }
        if (watch)
            loop.unwatch(watch);
        watch = 0;
        if (display) {
            wl_display_flush(display);
            wl_display_disconnect(display);
            display = nullptr;
        }
    }

    void failed()
    {
        if (!display)
            return;
        reset();
        q->availabilityChanged.emit();
    }

    // Protocol callbacks must finish before their listener data is freed.
    void failLater()
    {
        loop.post([this, alive = std::weak_ptr<bool>(alive)] {
            if (alive.lock())
                failed();
        });
    }

    void flush()
    {
        if (!display)
            return;
        if (wl_display_flush(display) < 0) {
            if (errno == EAGAIN)
                loop.setWatchEvents(watch, EPOLLIN | EPOLLOUT);
            else
                failLater();
        } else {
            loop.setWatchEvents(watch, EPOLLIN);
        }
    }

    void start()
    {
        // WAYLAND_DISPLAY, else wayland-0.
        display = wl_display_connect(nullptr);
        if (!display)
            return;
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
        static const wl_registry_listener registryListener = {
                [](void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
                    auto *self = static_cast<Private *>(data);
                    if (std::strcmp(interface, "wl_seat") == 0 && !self->seat)
                        self->seat = static_cast<wl_seat *>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
                    if (std::strcmp(interface, ext_data_control_manager_v1_interface.name) == 0) {
                        self->managerName = name;
                        self->managerVersion = std::min(version, 1u);
                    }
                },
                [](void *data, wl_registry *, uint32_t name) {
                    auto *self = static_cast<Private *>(data);
                    if (name == self->managerName && self->manager)
                        self->failLater();
                }};
        wl_registry_add_listener(registry, &registryListener, this);
        sync = wl_display_sync(display);
        static const wl_callback_listener ready = {[](void *data, wl_callback *callback, uint32_t) {
            auto *self = static_cast<Private *>(data);
            wl_callback_destroy(callback);
            self->sync = nullptr;
            self->bind();
        }};
        wl_callback_add_listener(sync, &ready, this);
        flush();
    }

    void bind()
    {
        if (!seat || !managerName)
            return;
        manager = static_cast<wl_proxy *>(
                wl_registry_bind(registry, managerName, &ext_data_control_manager_v1_interface, managerVersion));
        device = wl_proxy_marshal_flags(manager, 1, &ext_data_control_device_v1_interface, managerVersion, 0,
                                        nullptr, seat);
        static const ext_data_control_device_v1_listener listener = {
                [](void *data, ext_data_control_device_v1 *, ext_data_control_offer_v1 *proxy) {
                    auto *self = static_cast<Private *>(data);
                    auto offer = std::make_shared<Offer>();
                    offer->proxy = reinterpret_cast<wl_proxy *>(proxy);
                    static const ext_data_control_offer_v1_listener offerListener = {
                            [](void *data, ext_data_control_offer_v1 *, const char *mime) {
                                auto *offer = static_cast<Offer *>(data);
                                const std::string format = mime;
                                // SAVE_TARGETS is a clipboard-manager handshake, not a payload.
                                if (format == "SAVE_TARGETS")
                                    return;
                                if (std::find(offer->formats.begin(), offer->formats.end(), format)
                                    != offer->formats.end())
                                    return;
                                if (offer->formats.size() >= kMaxFormats || format.size() > 1024) {
                                    offer->tooManyFormats = true;
                                    return;
                                }
                                offer->formats.push_back(format);
                            }};
                    wl_proxy_add_listener(offer->proxy,
                                          reinterpret_cast<void (**)(void)>(
                                                  const_cast<ext_data_control_offer_v1_listener *>(&offerListener)),
                                          offer.get());
                    self->pending[offer->proxy] = std::move(offer);
                },
                [](void *data, ext_data_control_device_v1 *, ext_data_control_offer_v1 *proxy) {
                    static_cast<Private *>(data)->select(reinterpret_cast<wl_proxy *>(proxy));
                },
                [](void *data, ext_data_control_device_v1 *) {
                    static_cast<Private *>(data)->failLater();
                },
                [](void *data, ext_data_control_device_v1 *, ext_data_control_offer_v1 *proxy) {
                    static_cast<Private *>(data)->pending.erase(reinterpret_cast<wl_proxy *>(proxy));
                }};
        wl_proxy_add_listener(device,
                              reinterpret_cast<void (**)(void)>(
                                      const_cast<ext_data_control_device_v1_listener *>(&listener)),
                              this);
        flush();
        q->availabilityChanged.emit();
    }

    void select(wl_proxy *proxy)
    {
        ++generation;
        reader.reset();
        collecting.reset();
        snapshot.reset();
        selection.reset();
        if (const auto it = pending.find(proxy); it != pending.end()) {
            selection = std::move(it->second);
            pending.erase(it);
        }
        if (!selection || selection->tooManyFormats) {
            q->dataChanged.emit();
            return;
        }
        const uint64_t expected = generation;
        collectTimeout.setSingleShot(true);
        collectTimeout.onTimeout = [this, expected] {
            if (expected != generation || !collecting)
                return;
            reader.reset();
            collecting.reset();
        };
        collectTimeout.start(kTransferTimeout);
        collecting = std::make_unique<ClipboardData>();
        formatIndex = 0;
        totalBytes = 0;
        readNext();
    }

    void readNext()
    {
        if (!selection)
            return;
        if (formatIndex >= selection->formats.size()) {
            snapshot = std::move(collecting);
            q->dataChanged.emit();
            return;
        }
        const std::string format = selection->formats[formatIndex++];
        int fds[2];
        if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) < 0) {
            collecting.reset();
            return;
        }
        wl_proxy_marshal_flags(selection->proxy, 0, nullptr, 1, 0, format.c_str(), fds[1]);
        close(fds[1]);
        const uint64_t expected = generation;
        reader = std::make_unique<PipeTransfer>(
                fds[0], false, std::string(), kMaxClipboardBytes - totalBytes,
                [this, expected, format](bool complete, std::string bytes) {
                    // The finished reader goes on the next turn, outside its own call.
                    loop.post([finished = std::shared_ptr<PipeTransfer>(std::move(reader))] {});
                    if (expected != generation)
                        return;
                    if (!complete) {
                        collecting.reset();
                        return;
                    }
                    totalBytes += bytes.size();
                    collecting->emplace_back(format, std::move(bytes));
                    readNext();
                });
        flush();
    }

    void publish(ClipboardData data)
    {
        if (!device || data.size() > kMaxFormats)
            return;
        auto next = std::make_unique<Source>();
        size_t size = 0;
        for (const auto &[format, bytes] : data) {
            size += bytes.size();
            if (size > kMaxClipboardBytes)
                return;
            next->bytes[format] = bytes;
        }
        next->proxy = wl_proxy_marshal_flags(manager, 0, &ext_data_control_source_v1_interface, 1, 0, nullptr);
        static const ext_data_control_source_v1_listener sourceListener = {
                [](void *data, ext_data_control_source_v1 *, const char *mime, int fd) {
                    auto *source = static_cast<Source *>(data);
                    std::erase_if(source->writes, [](const auto &write) { return write->isFinished(); });
                    const auto it = source->bytes.find(mime);
                    if (it == source->bytes.end() || source->writes.size() >= 16) {
                        close(fd);
                        return;
                    }
                    source->writes.push_back(
                            std::make_unique<PipeTransfer>(fd, true, it->second, kMaxClipboardBytes, nullptr));
                },
                [](void *data, ext_data_control_source_v1 *) {
                    auto *source = static_cast<Source *>(data);
                    source->bytes.clear();
                }};
        wl_proxy_add_listener(next->proxy,
                              reinterpret_cast<void (**)(void)>(
                                      const_cast<ext_data_control_source_v1_listener *>(&sourceListener)),
                              next.get());
        // Offered in the order the selection had them.
        for (const auto &[format, bytes] : data)
            wl_proxy_marshal_flags(next->proxy, 0, nullptr, 1, 0, format.c_str());
        wl_proxy_marshal_flags(device, 0, nullptr, wl_proxy_get_version(device), 0, next->proxy);
        source = std::move(next);
        snapshot = std::make_unique<ClipboardData>(std::move(data));
        flush();
    }

    WaylandClipboard *q;
    EventLoop &loop;
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_callback *sync = nullptr;
    wl_seat *seat = nullptr;
    wl_proxy *manager = nullptr;
    wl_proxy *device = nullptr;
    uint32_t managerName = 0;
    uint32_t managerVersion = 0;
    int watch = 0;
    std::map<wl_proxy *, std::shared_ptr<Offer>> pending;
    std::shared_ptr<Offer> selection;
    std::unique_ptr<Source> source;
    std::unique_ptr<ClipboardData> snapshot;
    std::unique_ptr<ClipboardData> collecting;
    std::unique_ptr<PipeTransfer> reader;
    Timer collectTimeout;
    uint64_t generation = 0;
    size_t totalBytes = 0;
    size_t formatIndex = 0;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
};

WaylandClipboard::WaylandClipboard()
    : d(std::make_unique<Private>(this))
{
}

WaylandClipboard::~WaylandClipboard() = default;

bool WaylandClipboard::isAvailable() const
{
    return d->device != nullptr;
}

const ClipboardData *WaylandClipboard::data() const
{
    return d->snapshot.get();
}

void WaylandClipboard::setData(ClipboardData data)
{
    d->publish(std::move(data));
}

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/shell_drag.h"

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "clipboard/selection.h"
#include "compositor/buffer.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "input/drag.h"
#include "input/seat.h"
#include "shell/shell.h"
#include "shell/shell_input.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"

namespace Lingmo::Shell {

namespace {

// What a drop may read: a few file lists or a picture, not a disk image.
constexpr size_t kMaxDropData = 16 * 1024 * 1024;

// Writes `data` into `fd` from the main loop, then closes it; a reader that
// goes away ends it early.
void writeAsync(int fd, std::string data)
{
    const int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    struct Writer {
        int fd;
        std::string data;
        size_t written = 0;
        int watch = 0;
    };
    auto writer = std::make_shared<Writer>(Writer {fd, std::move(data)});
    const auto pump = [writer] {
        while (writer->written < writer->data.size()) {
            const ssize_t count = ::write(writer->fd, writer->data.data() + writer->written,
                                          writer->data.size() - writer->written);
            if (count > 0) {
                writer->written += size_t(count);
                continue;
            }
            if (count < 0 && errno == EINTR)
                continue;
            if (count < 0 && errno == EAGAIN)
                return false;
            break;
        }
        return true;
    };
    if (pump()) {
        close(fd);
        return;
    }
    writer->watch = core::mainLoop()->watch(fd, EPOLLOUT, [writer, pump](uint32_t) {
        if (!pump())
            return;
        core::mainLoop()->unwatch(writer->watch);
        close(writer->fd);
    });
    if (!writer->watch)
        close(fd);
}

// Reads `fd` to its end from the main loop, then hands over what came.
void readAsync(int fd, std::function<void(std::string data)> done)
{
    const int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    struct Reader {
        int fd;
        std::string data;
        int watch = 0;
        std::function<void(std::string)> done;
    };
    auto reader = std::make_shared<Reader>(Reader {fd, {}, 0, std::move(done)});
    reader->watch = core::mainLoop()->watch(fd, EPOLLIN, [reader](uint32_t) {
        char chunk[16384];
        for (;;) {
            const ssize_t count = ::read(reader->fd, chunk, sizeof(chunk));
            if (count > 0 && reader->data.size() + size_t(count) <= kMaxDropData) {
                reader->data.append(chunk, size_t(count));
                continue;
            }
            if (count < 0 && errno == EINTR)
                continue;
            if (count < 0 && errno == EAGAIN)
                return;
            // The end, an error, or more than a drop may carry.
            const bool complete = count == 0;
            core::mainLoop()->unwatch(reader->watch);
            close(reader->fd);
            auto callback = std::move(reader->done);
            callback(complete ? std::move(reader->data) : std::string());
            return;
        }
    });
    if (!reader->watch) {
        close(fd);
        reader->done({});
    }
}

uint32_t lowestAction(uint32_t actions)
{
    return actions & (~actions + 1);
}

// The drag icon: the source's bitmap, uploaded like an shm buffer.
class BitmapBuffer final : public compositor::Buffer
{
public:
    explicit BitmapBuffer(ui::Bitmap bitmap)
        : m_bitmap(std::move(bitmap))
    {
    }

    base::Size size() const override { return m_bitmap.size(); }
    render::PixelFormat format() const override
    {
        return m_bitmap.format() == ui::Bitmap::Format::Argb32Premultiplied
                ? render::PixelFormat::Bgra8888
                : render::PixelFormat::Rgba8888;
    }
    const uint8_t *pixels() const override { return m_bitmap.constBits(); }
    int stride() const override { return int(m_bitmap.stride()); }
    bool retainsClientStorage() const override { return false; }

private:
    ui::Bitmap m_bitmap;
};

} // namespace

// A shell view's drag, as a client's wl_data_source.
class ShellDrag::DataSource final : public clipboard::DataSource
{
public:
    explicit DataSource(ui::DragSource source)
        : m_source(std::move(source))
    {
        for (const std::string &mime : m_source.mimeTypes)
            offer(mime);
    }

    ~DataSource() override { finish(ui::NoAction); }

    void send(const std::string &mime, int fd) override
    {
        writeAsync(fd, m_source.data ? m_source.data(mime) : std::string());
    }

    void action(uint32_t action) override { m_action = action; }

    void finished() override { finish(ui::DropAction(m_action)); }

    void finish(ui::DropAction action)
    {
        if (auto callback = std::exchange(m_source.onFinished, nullptr))
            callback(action);
    }

protected:
    void cancelled() override { finish(ui::NoAction); }

private:
    ui::DragSource m_source;
    uint32_t m_action = 0;
};

// A drag over a shell window, as its views see it.
class ShellDrag::Offer final : public ui::DropOffer
{
public:
    Offer(ShellWindow *window, std::shared_ptr<input::DragOffer> offer)
        : window(window)
        , offer(std::move(offer))
    {
    }

    const std::vector<std::string> &mimeTypes() const override { return offer->formats(); }
    uint32_t sourceActions() const override { return offer->sourceActions(); }

    void read(const std::string &mimeType, std::function<void(std::string data)> done) override
    {
        int fds[2];
        if (!hasMimeType(mimeType) || pipe2(fds, O_CLOEXEC) != 0) {
            core::mainLoop()->post([done] { done({}); });
            return;
        }
        ++pendingReads;
        // The source may answer at once or later; either way from the loop.
        offer->receive(mimeType, fds[1]);
        readAsync(fds[0], [this, alive = alive, done](std::string data) {
            if (*alive)
                --pendingReads;
            done(std::move(data));
            if (*alive)
                finishIfDone();
        });
    }

    void finishIfDone()
    {
        if (dropped && pendingReads == 0 && !finished) {
            finished = true;
            offer->finish();
            if (onFinished)
                onFinished();
        }
    }

    ShellWindow *window;
    std::shared_ptr<input::DragOffer> offer;
    int pendingReads = 0;
    bool dropped = false;
    bool finished = false;
    std::function<void()> onFinished;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

    ~Offer() override { *alive = false; }
};

ShellDrag::ShellDrag(Shell *shell)
    : m_shell(shell)
{
    input::DragController *drag = m_shell->seat()->drag();
    m_subscriptions.push_back(drag->entered.connect(
            [this](compositor::Surface *surface, const base::PointF &location) {
                enter(surface, location);
            }));
    m_subscriptions.push_back(drag->moved.connect(
            [this](input::Timestamp, const base::PointF &location) { motion(location); }));
    m_subscriptions.push_back(drag->left.connect([this] { leave(); }));
    m_subscriptions.push_back(
            drag->dropped.connect([this](input::DragOffer *offer) { dropped(offer); }));
    m_subscriptions.push_back(drag->ended.connect([this] {
        // The icon's content went to the return animation if it slides home.
        m_icon.reset();
        m_iconBuffer.reset();
    }));
}

ShellDrag::~ShellDrag()
{
    *m_alive = false;
    m_subscriptions.clear();
    input::DragController *drag = m_shell->seat()->drag();
    if (m_icon && drag->active())
        drag->cancel();
    m_offer.reset();
    m_dropped.clear();
    m_icon.reset();
    m_iconBuffer.reset();
}

bool ShellDrag::start(ShellWindow *window, ui::DragSource source)
{
    if (m_starting || m_shell->seat()->drag()->busy())
        return false;
    m_starting = true;
    core::mainLoop()->post([this, alive = m_alive, window, source = std::move(source)]() mutable {
        if (!*alive)
            return;
        m_starting = false;
        const auto &windows = m_shell->windows();
        if (std::find(windows.begin(), windows.end(), window) == windows.end()) {
            if (source.onFinished)
                source.onFinished(ui::NoAction);
            return;
        }
        begin(window, std::move(source));
    });
    return true;
}

void ShellDrag::begin(ShellWindow *window, ui::DragSource source)
{
    input::Seat *seat = m_shell->seat();
    input::DragController *drag = seat->drag();
    compositor::Surface *origin = window->surface();
    const std::optional<input::Seat::TouchPoint> touch = seat->touchPointOn(origin);
    const bool pointer = seat->pressedButtons() != input::NoButton
            && seat->pointerGrabSurface() == origin;
    if (drag->busy() || (!pointer && !touch)) {
        if (source.onFinished)
            source.onFinished(ui::NoAction);
        return;
    }

    if (!source.icon.isNull()) {
        m_icon = m_shell->compositor()->createSurface();
        const double scale = source.iconScale > 0 ? source.iconScale : 1;
        const base::Size logical(std::max(1, int(std::lround(source.icon.size().width() / scale))),
                                 std::max(1, int(std::lround(source.icon.size().height() / scale))));
        const base::Point hotSpot(int(std::lround(source.hotSpot.x)),
                                  int(std::lround(source.hotSpot.y)));
        m_iconBuffer = std::make_shared<BitmapBuffer>(std::move(source.icon));
        m_icon->attach(m_iconBuffer, -hotSpot);
        m_icon->setViewport(logical);
        m_icon->damage(base::Rect(base::Point(), logical));
        m_icon->commit();
    }
    const uint32_t actions = source.actions;
    auto data = std::make_shared<DataSource>(std::move(source));
    if (pointer)
        drag->start(origin, m_icon.get(), data, actions, input::LeftButton);
    else
        drag->startTouch(origin, m_icon.get(), data, actions, touch->id, touch->location);
    if (!drag->active()) {
        data->finish(ui::NoAction);
        m_icon.reset();
        m_iconBuffer.reset();
    }
}

ShellWindow *ShellDrag::target() const
{
    return m_offer ? m_offer->window : nullptr;
}

void ShellDrag::windowDestroying(ShellWindow *window)
{
    if (m_offer && m_offer->window == window)
        m_offer.reset();
    for (const auto &offer : m_dropped) {
        if (offer->window == window)
            offer->window = nullptr;
    }
}

void ShellDrag::enter(compositor::Surface *surface, const base::PointF &location)
{
    ShellWindow *window = m_shell->input()->windowForSurface(surface);
    input::DragController *drag = m_shell->seat()->drag();
    // A drag without data stays inside the client that started it.
    if (!window || !window->window() || !drag->hasSource())
        return;
    m_offer = std::make_shared<Offer>(window, drag->createOffer(false));
    apply(window->window()->dragEnter({location.x(), location.y()}, m_offer.get(),
                                      m_shell->input()->modifiers()));
}

void ShellDrag::motion(const base::PointF &location)
{
    if (!m_offer || !m_offer->window || !m_offer->window->window())
        return;
    apply(m_offer->window->window()->dragMotion({location.x(), location.y()},
                                               m_shell->input()->modifiers()));
}

void ShellDrag::leave()
{
    std::shared_ptr<Offer> offer = std::exchange(m_offer, nullptr);
    if (offer && offer->window && offer->window->window())
        offer->window->window()->dragLeave();
}

void ShellDrag::dropped(input::DragOffer *dropped)
{
    if (!m_offer || m_offer->offer.get() != dropped)
        return;
    std::shared_ptr<Offer> offer = m_offer;
    m_dropped.push_back(offer);
    offer->onFinished = [this, alive = m_alive, raw = offer.get()] {
        if (!*alive)
            return;
        // Not from inside the offer's own callback.
        core::mainLoop()->post([this, alive, raw] {
            if (*alive) {
                std::erase_if(m_dropped,
                              [raw](const std::shared_ptr<Offer> &kept) { return kept.get() == raw; });
            }
        });
    };
    offer->dropped = true;
    const uint32_t taken = offer->window && offer->window->window() ? offer->window->window()->drop()
                                                                    : ui::NoAction;
    if (taken == ui::NoAction) {
        offer->offer->invalidate();
        offer->finished = true;
        offer->onFinished();
        return;
    }
    offer->finishIfDone();
}

void ShellDrag::apply(uint32_t action)
{
    input::DragOffer *offer = m_offer ? m_offer->offer.get() : nullptr;
    if (!offer)
        return;
    const uint32_t chosen = lowestAction(action & offer->sourceActions());
    offer->setActions(chosen, chosen);
    offer->accept(chosen && !offer->formats().empty() ? offer->formats().front().c_str() : nullptr);
}

} // namespace Lingmo::Shell

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/screenshot/screen_grabber.h"

#include <chrono>
#include <list>

#include "base/event_loop.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "base/transform.h"
#include "compositor/capture_source.h"
#include "ui/bitmap_painter.h"
#include "wayland/protocols/foreign_toplevel_list.h"
#include "wayland/protocols/image_capture.h"

namespace Lingmo::Shell {

static constexpr int kAttempts = 3;
static constexpr std::chrono::milliseconds kRetryDelay {50};
static constexpr std::chrono::milliseconds kTimeout {3000};

ui::Bitmap uprightImage(const ui::Bitmap &image, uint32_t transform)
{
    if (transform == 0 || image.isNull())
        return image;
    // Mirrored for the flipped transforms, then turned, and moved back into
    // view.
    base::AffineTransform matrix;
    if (transform & 4)
        matrix.scale(-1, 1);
    matrix.rotate(90 * int(transform & 3));
    const base::Rect bounds = matrix.mapRect(image.rect());
    matrix = matrix * base::AffineTransform::fromTranslate(-bounds.x(), -bounds.y());
    ui::Bitmap result(bounds.size(), image.format());
    result.fill(0);
    ui::BitmapPainter painter(result);
    painter.setCompositionMode(ui::BitmapPainter::CompositionMode::Source);
    painter.setTransform(matrix);
    painter.drawBitmap(base::RectF(0, 0, image.width(), image.height()), image, false);
    return result;
}

namespace {

class CompositorScreenGrabber final : public ScreenGrabber
{
public:
    CompositorScreenGrabber(wayland::ImageCaptureManager *capture, wayland::ForeignToplevelList *windows)
        : m_capture(capture)
        , m_windows(windows)
    {
    }

    void grabOutputs(const std::vector<std::string> &outputs, bool cursor, Done done) override
    {
        std::vector<std::shared_ptr<compositor::CaptureSource>> sources;
        for (const std::string &output : outputs)
            sources.push_back(m_capture ? wayland::ImageCaptureManager::outputSource(output) : nullptr);
        start(std::move(sources), cursor, std::move(done));
    }

    void grabWindow(wm::Window *window, bool cursor, Done done) override
    {
        std::shared_ptr<compositor::CaptureSource> source;
        if (m_capture && m_windows && window)
            source = m_windows->shadowedSourceFor(window);
        start({source}, cursor, std::move(done));
    }

    void grabSource(std::shared_ptr<compositor::CaptureSource> source, bool cursor, Done done) override
    {
        start({std::move(source)}, cursor, std::move(done));
    }

private:
    struct Slot {
        std::shared_ptr<compositor::CaptureSource> source;
        std::shared_ptr<void> handle;
        std::unique_ptr<base::Timer> retry;
        base::Job upright;
        int attempts = 0;
        bool finished = false;
    };

    struct Request {
        Done done;
        bool cursor = false;
        std::vector<Slot> slots;
        std::vector<ui::Bitmap> images;
        int remaining = 0;
        base::Timer timeout;
    };

    using Requests = std::list<Request>;

    void start(std::vector<std::shared_ptr<compositor::CaptureSource>> sources, bool cursor, Done done)
    {
        if (sources.empty()) {
            done({});
            return;
        }
        Request &request = m_requests.emplace_back();
        const Requests::iterator it = std::prev(m_requests.end());
        request.done = std::move(done);
        request.cursor = cursor;
        request.slots.resize(sources.size());
        request.images.resize(sources.size());
        request.remaining = int(sources.size());
        request.timeout.setSingleShot(true);
        request.timeout.onTimeout = [this, it] {
            for (size_t i = 0; i < it->slots.size(); ++i) {
                if (!it->slots[i].finished && complete(it, i, {}))
                    return;
            }
        };
        request.timeout.start(kTimeout);
        for (size_t i = 0; i < sources.size(); ++i)
            request.slots[i].source = std::move(sources[i]);
        // Every slot may finish at once; the last one ends the request.
        for (size_t i = 0, count = request.slots.size(); i < count; ++i) {
            if (ask(it, i))
                return;
        }
    }

    // True when that ended the request.
    bool ask(Requests::iterator it, size_t index)
    {
        Slot &slot = it->slots[index];
        if (!slot.source || !m_capture)
            return complete(it, index, {});
        ++slot.attempts;
        slot.handle = m_capture->capture(slot.source, it->cursor,
                                         [this, it, index](ui::Bitmap image, uint32_t transform) {
                                             received(it, index, std::move(image), transform);
                                         });
        return false;
    }

    void received(Requests::iterator it, size_t index, ui::Bitmap image, uint32_t transform)
    {
        Slot &slot = it->slots[index];
        slot.handle.reset();
        if (slot.finished)
            return;
        if (image.isNull()) {
            if (slot.attempts >= kAttempts) {
                complete(it, index, {});
                return;
            }
            slot.retry = std::make_unique<base::Timer>();
            slot.retry->setSingleShot(true);
            slot.retry->onTimeout = [this, it, index] { ask(it, index); };
            slot.retry->start(kRetryDelay);
            return;
        }
        if (transform == 0) {
            complete(it, index, std::move(image));
            return;
        }
        slot.upright = base::runInThread(
                [image = std::move(image), transform] { return uprightImage(image, transform); },
                [this, it, index](ui::Bitmap upright) { complete(it, index, std::move(upright)); });
    }

    // True when that ended the request. The request goes on the next loop
    // turn: this may run inside one of its own timers.
    bool complete(Requests::iterator it, size_t index, ui::Bitmap image)
    {
        Slot &slot = it->slots[index];
        if (slot.finished)
            return false;
        slot.finished = true;
        slot.handle.reset();
        it->images[index] = std::move(image);
        if (--it->remaining > 0)
            return false;
        it->timeout.stop();
        base::EventLoop::current()->post([this, it, alive = std::weak_ptr<bool>(m_alive)] {
            if (alive.expired())
                return;
            Done done = std::move(it->done);
            std::vector<ui::Bitmap> images = std::move(it->images);
            m_requests.erase(it);
            done(std::move(images));
        });
        return true;
    }

    wayland::ImageCaptureManager *m_capture;
    wayland::ForeignToplevelList *m_windows;
    Requests m_requests;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace

std::unique_ptr<ScreenGrabber> createScreenGrabber(wayland::ImageCaptureManager *capture,
                                                   wayland::ForeignToplevelList *windows)
{
    return std::make_unique<CompositorScreenGrabber>(capture, windows);
}

} // namespace Lingmo::Shell

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/animated_image.h"

#include <algorithm>
#include <cstring>
#include <optional>
#include <vector>

#include <webp/demux.h>

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"

#include "base/file.h"
#include "base/thread_pool.h"
#include "ui/color_profile.h"
#include "ui/frame_client.h"
#include "ui/gif_decoder.h"
#include "ui/image_decoder.h"
#include "ui/painter.h"

namespace Lingmo::ui {

// Frames kept in memory at most; a longer animation plays what fits.
static constexpr size_t kFrameBudget = size_t(512) * 1024 * 1024;

namespace {

struct Frames
{
    int width = 0;
    int height = 0;
    int loopCount = 0;
    std::vector<sk_sp<SkImage>> images;
    std::vector<int> delays;
};

} // namespace

static sk_sp<SkImage> imageFrom(const uint32_t* pixels, int width, int height,
                                const SourceProfile* profile = nullptr)
{
    const SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(info))
        return nullptr;
    bitmap.writePixels(SkPixmap(info, pixels, size_t(width) * 4));
    if (profile)
        profile->toSrgb(static_cast<uint8_t*>(bitmap.getPixels()), width, height, bitmap.rowBytes(), true, true);
    bitmap.setImmutable();
    return bitmap.asImage();
}

static Frames decodeGifFrames(const std::string& data)
{
    Frames frames;
    GifDecoder decoder;
    if (!decoder.open(data))
        return frames;
    frames.width = decoder.width();
    frames.height = decoder.height();
    // NETSCAPE 0 loops forever (-1), none plays once (0).
    frames.loopCount = decoder.loopCount() == 0 ? -1 : decoder.loopCount() < 0 ? 0 : decoder.loopCount();
    const size_t frameBytes = size_t(frames.width) * size_t(frames.height) * 4;
    const uint32_t* pixels = nullptr;
    int delay = 0;
    while (decoder.nextFrame(&pixels, &delay)) {
        if (!frames.images.empty() && (frames.images.size() + 1) * frameBytes > kFrameBudget)
            break;
        sk_sp<SkImage> image = imageFrom(pixels, frames.width, frames.height);
        if (!image)
            break;
        frames.images.push_back(std::move(image));
        frames.delays.push_back(delay);
    }
    return frames;
}

static Frames decodeWebpFrames(const std::string& data)
{
    Frames frames;
    WebPData webp = {reinterpret_cast<const uint8_t*>(data.data()), data.size()};
    WebPAnimDecoderOptions options;
    if (!WebPAnimDecoderOptionsInit(&options))
        return frames;
    // Premultiplied BGRA: the native 0xAARRGGBB words.
    options.color_mode = MODE_bgrA;
    options.use_threads = 0;
    WebPAnimDecoder* decoder = WebPAnimDecoderNew(&webp, &options);
    if (!decoder)
        return frames;
    // The decoder composes each frame onto the last, so its canvas stays as decoded.
    const std::unique_ptr<SourceProfile> profile = SourceProfile::fromWebp(data);
    WebPAnimInfo info;
    if (WebPAnimDecoderGetInfo(decoder, &info)) {
        frames.width = int(info.canvas_width);
        frames.height = int(info.canvas_height);
        // WebP's 0 is forever, -1 here.
        frames.loopCount = info.loop_count == 0 ? -1 : int(info.loop_count) - 1;
        const size_t frameBytes = size_t(frames.width) * size_t(frames.height) * 4;
        int previous = 0;
        while (WebPAnimDecoderHasMoreFrames(decoder)) {
            uint8_t* pixels = nullptr;
            int timestamp = 0;
            if (!WebPAnimDecoderGetNext(decoder, &pixels, &timestamp))
                break;
            if (!frames.images.empty() && (frames.images.size() + 1) * frameBytes > kFrameBudget)
                break;
            sk_sp<SkImage> image = imageFrom(reinterpret_cast<const uint32_t*>(pixels), frames.width, frames.height,
                                             profile.get());
            if (!image)
                break;
            frames.images.push_back(std::move(image));
            // A frame's duration, 10 ms or less stretched to 100.
            const int duration = timestamp - previous;
            frames.delays.push_back(duration <= 10 ? 100 : duration);
            previous = timestamp;
        }
    }
    WebPAnimDecoderDelete(decoder);
    return frames;
}

static Frames decodeFrames(const std::string& path)
{
    const std::optional<std::string> data = base::file::read(path);
    if (!data || data->empty())
        return {};
    if (data->starts_with("GIF8"))
        return decodeGifFrames(*data);
    if (data->size() > 12 && data->starts_with("RIFF") && data->compare(8, 4, "WEBP") == 0) {
        Frames frames = decodeWebpFrames(*data);
        if (!frames.images.empty())
            return frames;
    }
    // A still picture, as one frame.
    ImageRequest request;
    request.path = path;
    request.autoTransform = true;
    DecodedImage decoded = decodeImage(request);
    Frames frames;
    if (decoded.image) {
        frames.width = decoded.image->width();
        frames.height = decoded.image->height();
        frames.images.push_back(std::move(decoded.image));
        frames.delays.push_back(0);
    }
    return frames;
}

struct AnimatedImagePrivate final : FrameClient
{
    explicit AnimatedImagePrivate(Window* window, AnimatedImage* owner)
        : FrameClient(window)
        , q(owner)
    {
    }

    void tick(uint64_t frameTimeNs) override
    {
        if (!playing || frames.images.size() < 2) {
            stopTicking();
            return;
        }
        if (!lastNs) {
            lastNs = frameTimeNs;
            return;
        }
        elapsedMs += double(frameTimeNs - lastNs) / 1e6;
        lastNs = frameTimeNs;
        bool changed = false;
        while (elapsedMs >= frames.delays[size_t(current)]) {
            elapsedMs -= frames.delays[size_t(current)];
            if (current + 1 < int(frames.images.size())) {
                ++current;
            } else if (frames.loopCount < 0 || loopsDone < frames.loopCount) {
                ++loopsDone;
                current = 0;
            } else {
                playing = false;
                stopTicking();
                break;
            }
            changed = true;
        }
        if (changed && q->onFrameChanged)
            q->onFrameChanged();
    }

    void start()
    {
        lastNs = 0;
        if (playing && frames.images.size() > 1)
            startTicking();
    }

    AnimatedImage* q;
    Frames frames;
    bool loaded = false;
    bool playing = true;
    int current = 0;
    int loopsDone = 0;
    double elapsedMs = 0;
    uint64_t lastNs = 0;
    base::Job job;
};

AnimatedImage::AnimatedImage(Window* window)
    : d(std::make_unique<AnimatedImagePrivate>(window, this))
{
}

AnimatedImage::~AnimatedImage() = default;

void AnimatedImage::load(const std::string& path)
{
    d->loaded = false;
    d->frames = {};
    d->current = 0;
    d->loopsDone = 0;
    d->elapsedMs = 0;
    d->job = base::runInThread([path] { return decodeFrames(path); },
                               [this](Frames frames) {
                                   d->frames = std::move(frames);
                                   d->loaded = !d->frames.images.empty();
                                   d->start();
                                   if (onLoaded)
                                       onLoaded(d->loaded);
                                   if (d->loaded && onFrameChanged)
                                       onFrameChanged();
                               });
}

bool AnimatedImage::isLoaded() const
{
    return d->loaded;
}

base::Size AnimatedImage::size() const
{
    return {d->frames.width, d->frames.height};
}

int AnimatedImage::frameCount() const
{
    return int(d->frames.images.size());
}

int AnimatedImage::currentFrame() const
{
    return d->current;
}

int AnimatedImage::loopCount() const
{
    return d->frames.loopCount;
}

void AnimatedImage::setPlaying(bool playing)
{
    d->playing = playing;
    if (playing)
        d->start();
}

bool AnimatedImage::isPlaying() const
{
    return d->playing;
}

void AnimatedImage::jumpToFrame(int frame)
{
    if (frame < 0 || frame >= frameCount())
        return;
    d->current = frame;
    d->elapsedMs = 0;
    if (onFrameChanged)
        onFrameChanged();
}

void AnimatedImage::draw(Painter& painter, const RectF& rect, bool smooth) const
{
    if (!d->loaded)
        return;
    const SkImage* image = d->frames.images[size_t(d->current)].get();
    if (smooth) {
        painter.drawImage(image, rect);
        return;
    }
    const double scale = painter.scale();
    const SkRect target = SkRect::MakeXYWH(SkScalar((painter.offset().x + rect.x) * scale),
                                           SkScalar((painter.offset().y + rect.y) * scale),
                                           SkScalar(rect.width * scale), SkScalar(rect.height * scale));
    SkPaint paint;
    paint.setAlphaf(float(painter.opacity()));
    painter.canvas()->drawImageRect(image, target, SkSamplingOptions(), &paint);
}

} // namespace Lingmo::ui

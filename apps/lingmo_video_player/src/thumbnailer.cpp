// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailer.h"
#include "mpvwait.h"
#include "urls.h"

#include "base/elapsed_timer.h"
#include "base/paths.h"
#include "base/strings.h"
#include "base/uuid.h"

#include <mpv/client.h>
#include <mpv/render.h>

#include <algorithm>
#include <clocale>
#include <cmath>
#include <filesystem>

using namespace Lingmo;
namespace fs = std::filesystem;

static constexpr int kFrameWidth = 240;
static constexpr int kBucketCount = 150;

static void setOption(mpv_handle* mpv, const char* name, const char* value)
{
    mpv_set_option_string(mpv, name, value);
}

Thumbnailer::Thumbnailer(std::string coverFolder)
    : m_coverFolder(coverFolder.empty() ? defaultCoverFolder() : std::move(coverFolder))
{
    if (base::EventLoop* loop = base::EventLoop::current())
        m_loop = loop->handle();
    m_weak = m_weakFactory.getWeakPtr();
}

Thumbnailer::~Thumbnailer()
{
    {
        std::lock_guard lock(m_mutex);
        m_stopping = true;
        if (m_activeMpv)
            mpv_wakeup(m_activeMpv);
    }
    m_wake.notify_all();
    if (m_thread.joinable())
        m_thread.join();
    setCover({});
}

std::string Thumbnailer::defaultCoverFolder()
{
    return base::paths::cacheHome() + "/lingmo/lingmo-video-player/covers";
}

void Thumbnailer::setSource(const std::string& source)
{
    if (m_source == source)
        return;
    m_source = source;
    const bool local = urls::isLocalFile(source);
    {
        std::lock_guard lock(m_mutex);
        // Streams would download a second copy; only local files get frames.
        m_file = local ? urls::toLocalFile(source) : std::string();
        ++m_generation;
        m_pending = -1;
        m_coverWanted = false;
        m_frames.clear();
        if (m_activeMpv)
            mpv_wakeup(m_activeMpv);
    }
    m_wake.notify_all();
    // The thread starts with the first file that can have frames.
    if (!m_thread.joinable() && local) {
        // mpv needs the C locale for numbers.
        std::setlocale(LC_NUMERIC, "C");
        m_thread = std::thread([this] { run(); });
    }
    m_coverAsked = false;
    setCover({});
    askCover();
    sourceChanged.emit();
    ++m_revision;
    revisionChanged.emit();
}

void Thumbnailer::setDuration(double duration)
{
    if (std::abs(m_duration - duration) <= 1e-12 * std::max(1.0, std::abs(duration)))
        return;
    m_duration = duration;
    {
        std::lock_guard lock(m_mutex);
        m_sharedDuration = duration;
        m_step = step();
        m_frames.clear();
    }
    askCover();
    durationChanged.emit();
}

void Thumbnailer::askCover()
{
    {
        std::lock_guard lock(m_mutex);
        if (m_coverAsked || m_duration <= 0 || m_file.empty())
            return;
        m_coverAsked = true;
        m_coverWanted = true;
    }
    m_wake.notify_all();
}

double Thumbnailer::step() const
{
    return std::max(1.0, m_duration / kBucketCount);
}

int Thumbnailer::bucket(double seconds) const
{
    return std::clamp(int(std::floor(seconds / step())), 0, int(m_duration / step()));
}

double Thumbnailer::bucketTime(int bucket) const
{
    return bucket * step();
}

bool Thumbnailer::has(int bucket) const
{
    std::lock_guard lock(m_mutex);
    return m_frames.contains(bucket);
}

void Thumbnailer::request(int bucket)
{
    if (m_duration <= 0 || bucket < 0)
        return;
    {
        std::lock_guard lock(m_mutex);
        if (m_file.empty() || m_frames.contains(bucket) || m_pending == bucket)
            return;
        m_pending = bucket;
    }
    m_wake.notify_all();
}

ui::Image Thumbnailer::image(int bucket) const
{
    std::lock_guard lock(m_mutex);
    const auto found = m_frames.find(bucket);
    return found == m_frames.end() ? ui::Image() : found->second;
}

void Thumbnailer::addCover(const ui::Image& image, uint64_t generation)
{
    {
        std::lock_guard lock(m_mutex);
        if (generation != m_generation)
            return;
    }

    const fs::path folder(m_coverFolder);
    std::error_code error;
    // Covers left behind by a player that did not quit cleanly.
    if (!m_staleCoversRemoved) {
        m_staleCoversRemoved = true;
        fs::remove_all(folder, error);
    }
    fs::create_directories(folder, error);
    if (!fs::is_directory(folder, error))
        return;

    const int side = std::min(image.width(), image.height());
    const int left = (image.width() - side) / 2;
    const int top = (image.height() - side) / 2;
    ui::Image square(side, side);
    for (int y = 0; y < side; ++y) {
        const uint32_t* from = image.bits() + size_t(top + y) * size_t(image.width()) + size_t(left);
        std::copy(from, from + side, square.bits() + size_t(y) * size_t(side));
    }
    // A name never used before, also by an earlier player, as media controls cache images by URL.
    const std::string id = base::strings::replaceAll(base::Uuid::generate().toString(), "-", "");
    const fs::path path = folder / (id + ".png");
    if (square.savePng(path.string()))
        setCover(urls::fromLocalFile(path.string()));
}

void Thumbnailer::setCover(const std::string& cover)
{
    if (m_cover == cover)
        return;
    if (urls::isLocalFile(m_cover)) {
        std::error_code error;
        fs::remove(urls::toLocalFile(m_cover), error);
    }
    m_cover = cover;
    coverChanged.emit();
}

bool Thumbnailer::cancelled(uint64_t generation) const
{
    std::lock_guard lock(m_mutex);
    return m_stopping || generation != m_generation;
}

void Thumbnailer::addFrame(int bucket, ui::Image image, uint64_t generation, double step)
{
    {
        std::lock_guard lock(m_mutex);
        // A frame taken for another file or another length is not this bucket's.
        if (generation != m_generation || step != m_step)
            return;
        m_frames[bucket] = std::move(image);
    }
    ++m_revision;
    revisionChanged.emit();
}

void Thumbnailer::run()
{
    mpv_handle* mpv = nullptr;
    mpv_render_context* context = nullptr;
    uint64_t loadedGeneration = 0;
    int frameHeight = 0;

    uint64_t generation = 0;
    auto isCancelled = [&] { return cancelled(generation); };

    auto close = [&] {
        {
            std::lock_guard lock(m_mutex);
            m_activeMpv = nullptr;
        }
        if (context)
            mpv_render_context_free(context);
        context = nullptr;
        if (mpv)
            mpv_terminate_destroy(mpv);
        mpv = nullptr;
        frameHeight = 0;
    };

    auto render = [&](ui::Image& image) {
        int size[2] = {image.width(), image.height()};
        size_t stride = size_t(image.width()) * 4;
        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_SW_SIZE, size},
            {MPV_RENDER_PARAM_SW_FORMAT, const_cast<char*>("bgr0")},
            {MPV_RENDER_PARAM_SW_STRIDE, &stride},
            {MPV_RENDER_PARAM_SW_POINTER, image.bits()},
            {MPV_RENDER_PARAM_INVALID, nullptr},
        };
        return mpv_render_context_render(context, params) >= 0;
    };

    auto open = [&](const std::string& file) {
        mpv = mpv_create();
        if (!mpv)
            return;
        {
            std::lock_guard lock(m_mutex);
            m_activeMpv = mpv;
        }
        setOption(mpv, "vo", "libmpv");
        setOption(mpv, "ao", "null");
        setOption(mpv, "aid", "no");
        setOption(mpv, "sid", "no");
        setOption(mpv, "hwdec", "no");
        setOption(mpv, "pause", "yes");
        setOption(mpv, "keep-open", "always");
        setOption(mpv, "hr-seek", "no");
        setOption(mpv, "cache", "no");
        setOption(mpv, "osd-level", "0");
        setOption(mpv, "load-scripts", "no");
        setOption(mpv, "ytdl", "no");
        setOption(mpv, "vd-lavc-skiploopfilter", "all");
        setOption(mpv, "vd-lavc-fast", "yes");
        if (mpv_initialize(mpv) < 0)
            return close();

        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_SW)},
            {MPV_RENDER_PARAM_INVALID, nullptr},
        };
        if (mpv_render_context_create(&context, mpv, params) < 0) {
            context = nullptr;
            return close();
        }

        const char* command[] = {"loadfile", file.c_str(), nullptr};
        // The size is known once the first frame has configured the output. The
        // load's own restart follows; left queued, it would end the first seek.
        if (mpv_command(mpv, command) < 0 || !waitForMpvEvent(mpv, MPV_EVENT_FILE_LOADED, 5000, isCancelled)
                || !waitForMpvEvent(mpv, MPV_EVENT_VIDEO_RECONFIG, 5000, isCancelled)
                || !waitForMpvEvent(mpv, MPV_EVENT_PLAYBACK_RESTART, 5000, isCancelled))
            return close();
        int64_t width = 0;
        int64_t height = 0;
        mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &width);
        mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &height);
        if (width <= 0 || height <= 0)
            return close();
        frameHeight = std::max(1, int(std::lround(double(kFrameWidth) * double(height) / double(width))));
    };

    // The keyframe at or before seconds; null on failure.
    auto grab = [&](double seconds) -> ui::Image {
        const std::string target = base::strings::number(seconds, 'f', 3);
        const char* seek[] = {"seek", target.c_str(), "absolute+keyframes", nullptr};
        if (mpv_command(mpv, seek) < 0)
            return {};

        // mpv redraws the old frame while it seeks and hands over the new one
        // just before the restart, so the last frame drawn by then is the one.
        ui::Image image(kFrameWidth, frameHeight);
        bool drawn = false;
        bool restarted = false;
        base::ElapsedTimer timer;
        timer.start();
        while (!restarted) {
            if (timer.elapsedMs() > 3000 || isCancelled())
                return {};
            const mpv_event_id event = mpv_wait_event(mpv, 0.004)->event_id;
            if (event == MPV_EVENT_END_FILE || event == MPV_EVENT_SHUTDOWN)
                return {};
            restarted = event == MPV_EVENT_PLAYBACK_RESTART;
            if (mpv_render_context_update(context) & MPV_RENDER_UPDATE_FRAME)
                drawn = render(image);
        }
        if (!drawn)
            return {};
        uint32_t* pixels = image.bits();
        for (size_t i = 0, count = size_t(image.width()) * size_t(image.height()); i < count; ++i)
            pixels[i] |= 0xff000000u;
        return image;
    };

    for (;;) {
        std::string file;
        int bucket = -1;
        double step = 1;
        bool cover = false;
        {
            std::unique_lock lock(m_mutex);
            m_wake.wait(lock, [&] {
                return m_stopping || m_pending >= 0 || m_coverWanted || loadedGeneration != m_generation;
            });
            if (m_stopping)
                break;
            file = m_file;
            generation = m_generation;
            bucket = m_pending;
            step = m_step;
            m_pending = -1;
            cover = m_coverWanted;
            m_coverWanted = false;
        }

        if (generation != loadedGeneration) {
            close();
            loadedGeneration = generation;
            if (!file.empty())
                open(file);
        }
        if (!context || frameHeight <= 0)
            continue;

        if (cover) {
            double duration = 0;
            {
                // The length may have arrived after the file did.
                std::lock_guard lock(m_mutex);
                duration = m_sharedDuration;
            }
            ui::Image image = grab(duration / 10);
            if (!image.isNull()) {
                m_loop.post([weak = m_weak, image = std::move(image), generation] {
                    if (weak)
                        weak->addCover(image, generation);
                });
            }
        }
        if (bucket >= 0) {
            ui::Image image = grab(bucket * step);
            if (!image.isNull()) {
                m_loop.post([weak = m_weak, bucket, image = std::move(image), generation, step]() mutable {
                    if (weak)
                        weak->addFrame(bucket, std::move(image), generation, step);
                });
            }
        }
    }
    close();
}

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/thumbnails.h"

#include "base/event_loop.h"
#include "base/paths.h"
#include "base/thread_pool.h"

#include "lingmo/files/thumbnails/thumbnail_cache.h"

#include <list>
#include <map>
#include <tuple>
#include <vector>

namespace fm::thumbnails {

bool canThumbnail(const std::string& mimeType)
{
    return files::ThumbnailCache::self()->canThumbnail(mimeType);
}

bool isOnMount(const std::string& path)
{
    static const std::string mounts = base::paths::runtimeDirectory() + "/mounts/";
    return path.starts_with(mounts);
}

namespace {

struct Key
{
    std::string path;
    int64_t modified;
    int width;
    int height;

    auto tie() const { return std::tie(path, modified, width, height); }
    bool operator<(const Key& other) const { return tie() < other.tie(); }
};

struct Cache
{
    std::map<Key, ui::Bitmap> images;
    std::list<Key> order;
    std::map<Key, std::vector<std::function<void(const ui::Bitmap&)>>> waiting;
};

} // namespace

// About 50 MB of 256 px thumbnails.
static constexpr size_t kCacheSize = 200;

static Cache& cache()
{
    static Cache instance;
    return instance;
}

static base::ThreadPool& pool()
{
    static base::ThreadPool threads(2);
    return threads;
}

static void remember(const Key& key, const ui::Bitmap& image)
{
    Cache& c = cache();
    if (c.images.emplace(key, image).second)
        c.order.push_back(key);
    while (c.order.size() > kCacheSize) {
        c.images.erase(c.order.front());
        c.order.pop_front();
    }
}

ui::Bitmap cached(const std::string& path, int64_t modified, int width, int height)
{
    const Key key {path, modified, width, height};
    if (const auto found = cache().images.find(key); found != cache().images.end())
        return found->second;

    files::ThumbnailCache* thumbnails = files::ThumbnailCache::self();
    const base::Size size(width, height);
    if (!thumbnails->hasCachedThumbnail(path, size))
        return {};
    ui::Bitmap image = thumbnails->cachedThumbnail(path, size).image;
    if (!image.isNull())
        remember(key, image);
    return image;
}

void load(const std::string& path, int64_t modified, int width, int height,
          std::function<void(const ui::Bitmap&)> done)
{
    const Key key {path, modified, width, height};
    if (const auto found = cache().images.find(key); found != cache().images.end()) {
        done(found->second);
        return;
    }

    auto& waiting = cache().waiting[key];
    waiting.push_back(std::move(done));
    if (waiting.size() > 1)
        return;

    const base::LoopHandle loop = base::EventLoop::current()->handle();
    pool().post([key, loop] {
        ui::Bitmap image =
                files::ThumbnailCache::self()->requestThumbnail(key.path, base::Size(key.width, key.height)).image;
        loop.post([key, image] {
            if (!image.isNull())
                remember(key, image);
            auto callbacks = std::move(cache().waiting[key]);
            cache().waiting.erase(key);
            for (auto& callback : callbacks)
                callback(image);
        });
    });
}

} // namespace fm::thumbnails

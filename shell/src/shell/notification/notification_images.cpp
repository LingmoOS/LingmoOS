// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_images.h"

#include <algorithm>
#include <filesystem>
#include <set>
#include <system_error>

#include <sys/stat.h>

#include "base/paths.h"
#include "base/uuid.h"

namespace Lingmo::Shell {

namespace fs = std::filesystem;

static constexpr int32_t kMaximumSide = 4096;

std::optional<ImageHint> ImageHint::fromVariant(const base::dbus::Variant &hint)
{
    if (hint.signature() != "(iiibiiay)")
        return std::nullopt;
    const auto &items = hint.items();
    ImageHint image;
    image.width = items[0].value<int32_t>();
    image.height = items[1].value<int32_t>();
    image.rowStride = items[2].value<int32_t>();
    image.hasAlpha = items[3].value<bool>();
    const int32_t bitsPerSample = items[4].value<int32_t>();
    const int32_t channels = items[5].value<int32_t>();
    const std::string &pixels = items[6].bytes();

    if (image.width <= 0 || image.height <= 0 || image.width > kMaximumSide || image.height > kMaximumSide
        || bitsPerSample != 8 || channels != (image.hasAlpha ? 4 : 3) || image.rowStride < image.width * channels
        || int64_t(pixels.size()) < int64_t(image.rowStride) * (image.height - 1) + int64_t(image.width) * channels)
        return std::nullopt;

    image.pixels = pixels;
    return image;
}

ui::Bitmap ImageHint::toBitmap() const
{
    ui::Bitmap bitmap(width, height, ui::Bitmap::Format::Argb32Premultiplied);
    if (bitmap.isNull())
        return bitmap;
    const int channels = hasAlpha ? 4 : 3;
    for (int y = 0; y < height; ++y) {
        const auto *source = reinterpret_cast<const uint8_t *>(pixels.data()) + size_t(y) * size_t(rowStride);
        auto *target = reinterpret_cast<uint32_t *>(bitmap.scanLine(y));
        for (int x = 0; x < width; ++x, source += channels) {
            const uint32_t alpha = hasAlpha ? source[3] : 255;
            const auto premultiply = [alpha](uint32_t value) { return (value * alpha + 127) / 255; };
            target[x] = (alpha << 24) | (premultiply(source[0]) << 16) | (premultiply(source[1]) << 8)
                    | premultiply(source[2]);
        }
    }
    return bitmap;
}

namespace notification_images {

std::string defaultDirectory()
{
    // A directory of its own: ~/.cache/lingmo/notifications, an older
    // cache, is left alone.
    return base::paths::cacheHome() + "/lingmo/notification-images";
}

static std::string cleanPath(const std::string &path)
{
    std::string clean = fs::path(path).lexically_normal().string();
    while (clean.size() > 1 && clean.back() == '/')
        clean.pop_back();
    return clean;
}

std::string save(const std::string &directory, const ui::Bitmap &image)
{
    if (image.isNull())
        return {};

    std::error_code error;
    fs::create_directories(directory, error);
    // The pictures show what the notifications said: only the user may list
    // or read them.
    if (chmod(directory.c_str(), 0700) != 0)
        return {};

    ui::Bitmap thumbnail = image;
    if (image.width() > kThumbnailPixels && image.height() > kThumbnailPixels)
        thumbnail = image.scaled(ui::scaledSize(image.size(), base::Size(kThumbnailPixels, kThumbnailPixels),
                                                ui::Bitmap::Scaling::Cover));
    const int side = std::min(thumbnail.width(), thumbnail.height());
    thumbnail = thumbnail.copy(
            base::Rect((thumbnail.width() - side) / 2, (thumbnail.height() - side) / 2, side, side));

    const std::string path = directory + '/' + base::Uuid::generate().toString() + ".png";
    return thumbnail.savePng(path) ? path : std::string();
}

void remove(const std::string &directory, const std::string &path)
{
    if (path.empty())
        return;
    // Applications choose these paths: "../" must not lead out of the cache,
    // and the kernel would resolve it through symlinks, so the cleaned path
    // is what goes.
    const std::string cleaned = cleanPath(path);
    if (fs::path(cleaned).parent_path().string() != cleanPath(directory))
        return;
    std::error_code error;
    fs::remove(cleaned, error);
}

void prune(const std::string &directory, const std::vector<std::string> &keep)
{
    const std::set<std::string> kept(keep.begin(), keep.end());
    std::error_code error;
    for (const auto &item : fs::directory_iterator(directory, error)) {
        std::error_code typeError;
        if (!item.is_regular_file(typeError))
            continue;
        const std::string path = item.path().string();
        // Only what save() writes: a UUID and ".png".
        const std::string name = item.path().filename().string();
        if (name.size() != 36 + 4 || name.compare(36, 4, ".png") != 0
            || !base::Uuid::parse(name.substr(0, 36)))
            continue;
        if (!kept.count(path))
            fs::remove(item.path(), typeError);
    }
}

} // namespace notification_images

} // namespace Lingmo::Shell

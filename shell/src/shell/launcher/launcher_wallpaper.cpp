// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_wallpaper.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "shell/wallpaper/wallpaper.h"
#include "ui/bitmap.h"

namespace Lingmo::Shell {

// The black over a picture.
static constexpr double kDimmer = 0.4;

struct LauncherWallpaper::Result {
    ui::Image image;
    std::optional<ui::Color> average;
};

static void blurPass(const uint32_t *source, size_t sourceStride, uint32_t *result, size_t resultStride,
                     int width, int height, int radius, bool horizontal)
{
    const size_t sourceStep = horizontal ? 1 : sourceStride;
    const size_t resultStep = horizontal ? 1 : resultStride;
    const int length = horizontal ? width : height;
    const int lines = horizontal ? height : width;
    const int samples = radius * 2 + 1;
    for (int line = 0; line < lines; ++line) {
        const uint32_t *input = source + (horizontal ? size_t(line) * sourceStride : size_t(line));
        uint32_t *output = result + (horizontal ? size_t(line) * resultStride : size_t(line));
        const auto pixel = [&](int position) {
            return input[size_t(std::clamp(position, 0, length - 1)) * sourceStep];
        };
        int red = 0, green = 0, blue = 0, alpha = 0;
        const auto accumulate = [&](uint32_t value, int sign) {
            red += int((value >> 16) & 0xff) * sign;
            green += int((value >> 8) & 0xff) * sign;
            blue += int(value & 0xff) * sign;
            alpha += int(value >> 24) * sign;
        };
        for (int offset = -radius; offset <= radius; ++offset)
            accumulate(pixel(offset), 1);
        for (int position = 0; position < length; ++position) {
            output[size_t(position) * resultStep] = uint32_t(alpha / samples) << 24
                    | uint32_t(red / samples) << 16 | uint32_t(green / samples) << 8
                    | uint32_t(blue / samples);
            accumulate(pixel(position - radius), -1);
            accumulate(pixel(position + radius + 1), 1);
        }
    }
}

void LauncherWallpaper::blur(ui::Bitmap &image)
{
    if (image.isNull() || image.format() != ui::Bitmap::Format::Argb32Premultiplied)
        return;
    ui::Bitmap scratch(image.size(), ui::Bitmap::Format::Argb32Premultiplied);
    if (scratch.isNull())
        return;
    const int width = image.width();
    const int height = image.height();
    auto *pixels = reinterpret_cast<uint32_t *>(image.bits());
    auto *other = reinterpret_cast<uint32_t *>(scratch.bits());
    const size_t stride = image.stride() / 4;
    const size_t otherStride = scratch.stride() / 4;
    for (int pass = 0; pass < 3; ++pass) {
        blurPass(pixels, stride, other, otherStride, width, height, 3, true);
        blurPass(other, otherStride, pixels, stride, width, height, 3, false);
    }
}

// Ink::averageColor: premultiplied sums over the alpha sum.
static std::optional<ui::Color> averageColor(const ui::Bitmap &image)
{
    int64_t red = 0, green = 0, blue = 0, alpha = 0;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const uint32_t *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            red += (line[x] >> 16) & 0xff;
            green += (line[x] >> 8) & 0xff;
            blue += line[x] & 0xff;
            alpha += line[x] >> 24;
        }
    }
    if (alpha == 0)
        return std::nullopt;
    return ui::Color {uint8_t(red * 255 / alpha), uint8_t(green * 255 / alpha),
                      uint8_t(blue * 255 / alpha), 255};
}

static ui::Image toImage(const ui::Bitmap &bitmap)
{
    ui::Image image(bitmap.width(), bitmap.height());
    uint32_t *out = image.bits();
    for (int y = 0; y < bitmap.height(); ++y) {
        const auto *line = reinterpret_cast<const uint32_t *>(bitmap.constScanLine(y));
        for (int x = 0; x < bitmap.width(); ++x) {
            const uint32_t p = line[x];
            const uint32_t a = p >> 24;
            const auto channel = [a](uint32_t value) {
                return a == 0 ? 0u : std::min(255u, (value * 255 + a / 2) / a);
            };
            *out++ = a << 24 | channel((p >> 16) & 0xff) << 16 | channel((p >> 8) & 0xff) << 8
                    | channel(p & 0xff);
        }
    }
    return image;
}

LauncherWallpaper::LauncherWallpaper(Wallpaper *wallpaper)
    : m_wallpaper(wallpaper)
{
    if (m_wallpaper)
        m_backgroundChanged = m_wallpaper->backgroundChanged.connect([this] { refresh(); });
}

LauncherWallpaper::~LauncherWallpaper() = default;

void LauncherWallpaper::setSurfaceSize(const base::Size &size)
{
    if (size == m_size)
        return;
    m_size = size;
    refresh();
}

void LauncherWallpaper::refresh()
{
    if (m_size.isEmpty())
        return;
    if (!m_wallpaper) {
        m_loaded = true;
        m_picture = false;
        changed();
        return;
    }
    const Wallpaper::Background &background = m_wallpaper->background();
    if (background.type != 0) {
        m_request.reset();
        m_job.cancel();
        m_pendingKey.clear();
        m_key.clear();
        const bool same = m_loaded && !m_picture && m_color == background.color;
        m_loaded = true;
        m_picture = false;
        m_color = background.color;
        m_image = {};
        m_average.reset();
        if (!same)
            changed();
        return;
    }

    // The desktop's decode at this size, shared and usually in memory: the
    // launcher covers the same output.
    const std::string key = background.path + '\n' + std::to_string(m_size.width()) + 'x'
            + std::to_string(m_size.height());
    if (key == m_key || key == m_pendingKey)
        return;
    m_pendingKey = key;
    m_job.cancel();
    m_request = m_wallpaper->cache().load(background.path, m_size,
                                          [this, key](const ui::Bitmap &image) { decoded(key, image); });
}

void LauncherWallpaper::decoded(const std::string &key, const ui::Bitmap &image)
{
    const base::Size target(std::max(1, int(std::lround(m_size.width() * kQuality))),
                            std::max(1, int(std::lround(m_size.height() * kQuality))));
    m_job = base::ThreadPool::global().run(
            [image, target] { return std::shared_ptr<Result>(shrink(image, target)); },
            [this, key](std::shared_ptr<Result> result) {
                m_pendingKey.clear();
                m_key = key;
                m_loaded = true;
                m_picture = true;
                m_image = std::move(result->image);
                m_average = result->average;
                changed();
            });
}

ui::Color LauncherWallpaper::backdropColor() const
{
    if (!m_picture)
        return m_color;
    const ui::Color base = m_average.value_or(ui::Color {0, 0, 0, 255});
    const auto dim = [](uint8_t channel) { return uint8_t(std::lround(channel * (1 - kDimmer))); };
    return {dim(base.r), dim(base.g), dim(base.b), 255};
}

LauncherWallpaper::Result *LauncherWallpaper::shrink(const ui::Bitmap &decode, const base::Size &target)
{
    auto *result = new Result;
    if (decode.isNull())
        return result;
    // Scaled to cover, centre-cropped, blurred.
    const base::Size covering = ui::scaledSize(decode.size(), target, ui::Bitmap::Scaling::Cover);
    ui::Bitmap image = decode.scaled(base::Size(std::max(covering.width(), target.width()),
                                                std::max(covering.height(), target.height())));
    image = image.copy(base::Rect((image.width() - target.width()) / 2,
                                  (image.height() - target.height()) / 2, target.width(),
                                  target.height()))
                    .converted(ui::Bitmap::Format::Argb32Premultiplied);
    LauncherWallpaper::blur(image);
    result->average = averageColor(image);
    result->image = toImage(image);
    return result;
}

} // namespace Lingmo::Shell

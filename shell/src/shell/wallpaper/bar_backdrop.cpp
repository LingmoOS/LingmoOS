// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/wallpaper/bar_backdrop.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

#include "ui/bitmap.h"

namespace Lingmo::Shell {

// Width the strip is averaged at; the detail thresholds hold for it.
static constexpr int kSampleWidth = 256;
// Foliage, rock and other fine texture sit above this; skies, water and flat
// artwork far below.
static constexpr double kBusyDetail = 8.0;
// The dark gradient lifts light contents, so a busy strip keeps them up to a
// lighter average than a smooth one.
static constexpr double kBusyLightForegroundLuminance = 0.45;
// Dark contents on a smooth background this light need no gradient.
static constexpr double kPaleLuminance = 0.6;

double BarBackdrop::relativeLuminance(double red, double green, double blue)
{
    const auto linear = [](double value) {
        value /= 255.0;
        return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * linear(red) + 0.7152 * linear(green) + 0.0722 * linear(blue);
}

bool BarBackdrop::prefersWhite(double luminance)
{
    return 1.05 / (luminance + 0.05) >= 3.0;
}

// qGray: (11 r + 16 g + 5 b) / 32.
static int grey(uint32_t pixel)
{
    return int((((pixel >> 16) & 0xff) * 11 + ((pixel >> 8) & 0xff) * 16 + (pixel & 0xff) * 5) / 32);
}

BarBackdrop::Sample BarBackdrop::sampleImage(const ui::Bitmap &screenImage, int barHeight)
{
    Sample sample;
    if (screenImage.isNull())
        return sample;

    const int rows = std::clamp(barHeight, 1, screenImage.height());
    const ui::Bitmap strip = screenImage.copy(base::Rect(0, 0, screenImage.width(), rows));
    const int width = std::min(kSampleWidth, strip.width());
    const int height = std::clamp(int(std::lround(double(width) * strip.height() / strip.width())), 1,
                                  strip.height());
    const ui::Bitmap image = strip.scaled(width, height, true).converted(ui::Bitmap::Format::Argb32Premultiplied);

    int64_t red = 0;
    int64_t green = 0;
    int64_t blue = 0;
    int64_t detail = 0;
    int64_t detailCount = 0;
    const uint32_t *previous = nullptr;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const uint32_t *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            // RGB32: opaque, so premultiplied is plain.
            const uint32_t pixel = line[x] | 0xff000000u;
            red += (pixel >> 16) & 0xff;
            green += (pixel >> 8) & 0xff;
            blue += pixel & 0xff;
            if (x > 0) {
                detail += std::abs(grey(pixel) - grey(line[x - 1]));
                ++detailCount;
            }
            if (previous) {
                detail += std::abs(grey(pixel) - grey(previous[x]));
                ++detailCount;
            }
        }
        previous = line;
    }
    const int64_t pixels = int64_t(image.width()) * image.height();
    if (pixels == 0)
        return sample;

    // The mean colour is judged, not the mean luminance: a few bright
    // pixels would outvote the whole strip.
    sample.valid = true;
    sample.red = double(red) / pixels;
    sample.green = double(green) / pixels;
    sample.blue = double(blue) / pixels;
    sample.detail = detailCount > 0 ? double(detail) / detailCount : 0.0;
    return sample;
}

BarBackdrop::Sample BarBackdrop::sampleColor(const ui::Color &color)
{
    Sample sample;
    sample.valid = true;
    sample.red = color.r;
    sample.green = color.g;
    sample.blue = color.b;
    return sample;
}

BarBackdrop::Style BarBackdrop::styleForSample(const Sample &sample)
{
    Style style;
    if (!sample.valid)
        return style;
    const double luminance = relativeLuminance(sample.red, sample.green, sample.blue);
    if (sample.detail >= kBusyDetail) {
        style.useLightForeground = luminance < kBusyLightForegroundLuminance;
        style.gradientVisible = true;
    } else {
        style.useLightForeground = prefersWhite(luminance);
        style.gradientVisible = !style.useLightForeground && luminance < kPaleLuminance;
    }
    if (style.gradientVisible) {
        style.gradientColor = style.useLightForeground ? ui::Color {0, 0, 0, 77} : ui::Color {255, 255, 255, 115};
    }
    return style;
}

} // namespace Lingmo::Shell

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include "ui/geometry.h"

namespace Lingmo::ui {
class Bitmap;
}

namespace Lingmo::Shell {

// The wallpaper's strip under the
// status bar, averaged, decides whether the bar's contents are light or dark
// and whether the desktop draws a gradient under the bar to lift them.
class BarBackdrop
{
public:
    // The status bar's height; the sample is its strip of the wallpaper.
    static constexpr int kBarHeight = 32;
    // The gradient under the bar, three bars tall.
    static constexpr int kGradientHeight = kBarHeight * 3;

    struct Sample {
        bool valid = false;
        double red = 0;
        double green = 0;
        double blue = 0;
        // Mean grey difference between neighbouring pixels of the strip.
        double detail = 0;
    };

    struct Style {
        bool useLightForeground = false;
        bool gradientVisible = false;
        ui::Color gradientColor;

        bool operator==(const Style &) const = default;
    };

    // `screenImage` covers the screen in device pixels; `barHeight` is the
    // bar's height in the same pixels.
    static Sample sampleImage(const ui::Bitmap &screenImage, int barHeight);
    static Sample sampleColor(const ui::Color &color);
    static Style styleForSample(const Sample &sample);
    // WCAG relative luminance of sRGB components 0..255, and whether white
    // keeps a contrast of 3:1 against it.
    static double relativeLuminance(double red, double green, double blue);
    static bool prefersWhite(double luminance);
};

} // namespace Lingmo::Shell


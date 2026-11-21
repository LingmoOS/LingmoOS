// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <vector>

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"

namespace Lingmo::ui {

// Premultiplied RGBA in floats, sampled as a linearly filtered texture
// clamped at its edges.
struct Texture
{
    int width = 0;
    int height = 0;
    std::vector<float> pixels;

    // Texels already within the texture.
    void blend(int x0, int x1, float fx, int y0, int y1, float fy, float weight, float* out) const
    {
        const float* row0 = &pixels[size_t(y0) * size_t(width) * 4];
        const float* row1 = &pixels[size_t(y1) * size_t(width) * 4];
        const float* a = row0 + x0 * 4;
        const float* b = row0 + x1 * 4;
        const float* c = row1 + x0 * 4;
        const float* d = row1 + x1 * 4;
        for (int i = 0; i < 4; ++i) {
            const float top = a[i] + (b[i] - a[i]) * fx;
            const float bottom = c[i] + (d[i] - c[i]) * fx;
            out[i] += (top + (bottom - top) * fy) * weight;
        }
    }
};

// Where a linearly filtered texture of one length is sampled for each
// pixel of an output of another, both covering the same quad.
struct Taps
{
    std::vector<int> first;
    std::vector<int> second;
    std::vector<float> fraction;

    // offset in normalised texture coordinates.
    Taps(int output, int texture, double offset = 0)
    {
        for (int i = 0; i < output; ++i) {
            const double t = ((i + 0.5) / output + offset) * texture - 0.5;
            const int low = int(std::floor(t));
            first.push_back(std::clamp(low, 0, texture - 1));
            second.push_back(std::clamp(low + 1, 0, texture - 1));
            fraction.push_back(float(t - low));
        }
    }
};

Texture readTexture(const SkPixmap& pixmap);
// The texture size for an item: whole logical pixels, then the device
// pixel ratio.
int textureLength(double logical, double scale);
// One blur level of the logical size over the source.
Texture blurPass(const Texture& source, double logicalWidth, double logicalHeight, double scale);
std::array<double, 6> multiEffectBlurWeights(double blurMax, double blur);
// How many blur levels the blurMax takes.
int multiEffectBlurLevels(double blurMax);
// The effect's own pass: the source and the blur items, weighted, for the
// rows from first to last.
SkBitmap composite(const Texture& source, const std::vector<Texture>& blurs, const std::array<double, 6>& weights,
                   int firstRow, int lastRow);

} // namespace Lingmo::ui

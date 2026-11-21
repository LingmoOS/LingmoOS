// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/multi_effect_blur.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include "include/core/SkImageInfo.h"

namespace Lingmo::ui {

Texture readTexture(const SkPixmap& pixmap)
{
    static const std::array<float, 256> kLevels = [] {
        std::array<float, 256> levels;
        for (int i = 0; i < 256; ++i)
            levels[size_t(i)] = i / 255.0f;
        return levels;
    }();
    Texture texture;
    texture.width = pixmap.width();
    texture.height = pixmap.height();
    texture.pixels.resize(size_t(texture.width) * size_t(texture.height) * 4);
    const bool bgra = pixmap.colorType() == kBGRA_8888_SkColorType;
    float* out = texture.pixels.data();
    for (int y = 0; y < texture.height; ++y) {
        const auto* row = static_cast<const uint8_t*>(pixmap.addr(0, y));
        for (int x = 0; x < texture.width; ++x, row += 4, out += 4) {
            out[0] = kLevels[row[bgra ? 2 : 0]];
            out[1] = kLevels[row[1]];
            out[2] = kLevels[row[bgra ? 0 : 2]];
            out[3] = kLevels[row[3]];
        }
    }
    return texture;
}

// The texture size for an item: whole logical pixels, then the device
// pixel ratio.
int textureLength(double logical, double scale)
{
    return std::max(1, int(std::lround(std::ceil(logical - 1e-6) * scale)));
}

// One blur level: four taps a logical pixel of the item away, the
// second axis a third as far, averaged.
Texture blurPass(const Texture& source, double logicalWidth, double logicalHeight, double scale)
{
    Texture result;
    result.width = textureLength(logicalWidth, scale);
    result.height = textureLength(logicalHeight, scale);
    result.pixels.assign(size_t(result.width) * size_t(result.height) * 4, 0.0f);
    const double ox = 1 / logicalWidth;
    const double oy = 1 / logicalHeight;
    const double dither = 0.33;
    // The four taps' offsets, as bluritems.vert places them.
    const double offsets[4][2] = {
        {ox, oy * dither},
        {ox * dither, -oy},
        {-ox * dither, oy},
        {-ox, -oy * dither},
    };
    std::vector<Taps> columns;
    std::vector<Taps> rows;
    for (const auto& offset : offsets) {
        columns.emplace_back(result.width, source.width, offset[0]);
        rows.emplace_back(result.height, source.height, offset[1]);
    }
    for (int y = 0; y < result.height; ++y) {
        const float* top[4];
        const float* bottom[4];
        float fy[4];
        for (size_t k = 0; k < 4; ++k) {
            top[k] = &source.pixels[size_t(rows[k].first[size_t(y)]) * size_t(source.width) * 4];
            bottom[k] = &source.pixels[size_t(rows[k].second[size_t(y)]) * size_t(source.width) * 4];
            fy[k] = rows[k].fraction[size_t(y)];
        }
        float* out = &result.pixels[size_t(y) * size_t(result.width) * 4];
        for (int x = 0; x < result.width; ++x, out += 4) {
            // Summed in registers, in the order the taps are listed.
            float sum[4] = {0, 0, 0, 0};
            for (size_t k = 0; k < 4; ++k) {
                const float* a = top[k] + columns[k].first[size_t(x)] * 4;
                const float* b = top[k] + columns[k].second[size_t(x)] * 4;
                const float* c = bottom[k] + columns[k].first[size_t(x)] * 4;
                const float* d = bottom[k] + columns[k].second[size_t(x)] * 4;
                const float fx = columns[k].fraction[size_t(x)];
                for (int i = 0; i < 4; ++i) {
                    const float upper = a[i] + (b[i] - a[i]) * fx;
                    const float lower = c[i] + (d[i] - c[i]) * fx;
                    sum[i] += (upper + (lower - upper) * fy[k]) * 0.25f;
                }
            }
            for (int i = 0; i < 4; ++i)
                out[i] = sum[i];
        }
    }
    return result;
}

// The weights of the source and the blur levels, by how near each level lies to
// the blur's level of detail (calculateLod). The six levels are normalised
// together whatever the effect's blur level; levels it lacks get nothing.
std::array<double, 6> multiEffectBlurWeights(double blurMax, double blur)
{
    const double lod = std::sqrt(blur * blurMax / 64) * 1.2 - 0.2;
    std::array<double, 6> weights;
    double sum = 0;
    for (size_t i = 0; i < weights.size(); ++i) {
        weights[i] = std::clamp(1 - std::fabs(lod - (0.1 + 0.2 * double(i))) * 2, 0.0, 1.0);
        sum += weights[i];
    }
    for (double& weight : weights)
        weight /= sum;
    return weights;
}

int multiEffectBlurLevels(double blurMax)
{
    if (blurMax > 32)
        return 5;
    if (blurMax > 16)
        return 4;
    if (blurMax > 8)
        return 3;
    if (blurMax > 2)
        return 2;
    return 1;
}

// The effect's own pass over the item's quad: the source and the blur
// items, weighted, for the rows from first to last.
SkBitmap composite(const Texture& source, const std::vector<Texture>& blurs, const std::array<double, 6>& weights,
                   int firstRow, int lastRow)
{
    std::vector<const Texture*> textures = {&source};
    for (const Texture& blur : blurs)
        textures.push_back(&blur);
    std::vector<Taps> columns;
    std::vector<Taps> rows;
    for (const Texture* texture : textures) {
        columns.emplace_back(source.width, texture->width);
        rows.emplace_back(source.height, texture->height);
    }

    // Linear filtering is separable: each texture's rows widened to the
    // output first, then each output row blended down the columns, texture
    // by texture into the sum. Column by column of tiles, which stay in cache.
    const int width = source.width;
    const int height = std::max(1, lastRow - firstRow);
    struct Used
    {
        size_t index;
        // The texture rows the output rows read.
        int firstRead;
        int lastRead;
    };
    std::vector<Used> used;
    for (size_t i = 0; i < textures.size(); ++i) {
        if (weights[i] <= 0)
            continue;
        int firstRead = textures[i]->height;
        int lastRead = 0;
        for (int y = firstRow; y < lastRow; ++y) {
            firstRead = std::min(firstRead, rows[i].first[size_t(y)]);
            lastRead = std::max(lastRead, rows[i].second[size_t(y)] + 1);
        }
        used.push_back({i, firstRead, lastRead});
    }

    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    constexpr int kTileWidth = 128;
    std::vector<std::vector<float>> widened(textures.size());
    std::vector<float> sum(size_t(kTileWidth) * 4);
    for (int x0 = 0; x0 < width; x0 += kTileWidth) {
        const int tile = std::min(kTileWidth, width - x0);
        for (const Used& use : used) {
            const Texture& texture = *textures[use.index];
            const Taps& column = columns[use.index];
            widened[use.index].resize(size_t(tile) * size_t(texture.height) * 4);
            for (int ty = use.firstRead; ty < use.lastRead; ++ty) {
                const float* in = &texture.pixels[size_t(ty) * size_t(texture.width) * 4];
                float* out = &widened[use.index][size_t(ty) * size_t(tile) * 4];
                for (int x = x0; x < x0 + tile; ++x, out += 4) {
                    const float* a = in + column.first[size_t(x)] * 4;
                    const float* b = in + column.second[size_t(x)] * 4;
                    const float f = column.fraction[size_t(x)];
                    float texel[4];
                    for (int c = 0; c < 4; ++c)
                        texel[c] = a[c] + (b[c] - a[c]) * f;
                    for (int c = 0; c < 4; ++c)
                        out[c] = texel[c];
                }
            }
        }
        for (int y = firstRow; y < lastRow; ++y) {
            std::fill(sum.begin(), sum.end(), 0.0f);
            for (const Used& use : used) {
                const std::vector<float>& rowsOfTile = widened[use.index];
                const Taps& row = rows[use.index];
                const float* top = &rowsOfTile[size_t(row.first[size_t(y)]) * size_t(tile) * 4];
                const float* bottom = &rowsOfTile[size_t(row.second[size_t(y)]) * size_t(tile) * 4];
                const float f = row.fraction[size_t(y)];
                const float weight = float(weights[use.index]);
                float* out = sum.data();
                for (int k = 0; k < tile * 4; ++k)
                    out[k] += (top[k] + (bottom[k] - top[k]) * f) * weight;
            }
            auto* out = static_cast<uint8_t*>(bitmap.getAddr(x0, y - firstRow));
            for (int k = 0; k < tile * 4; ++k)
                out[k] = uint8_t(int(std::min(std::max(sum[size_t(k)], 0.0f), 1.0f) * 255 + 0.5f));
        }
    }
    // No rows asked for: one transparent row.
    if (lastRow <= firstRow)
        bitmap.eraseColor(SK_ColorTRANSPARENT);
    bitmap.setImmutable();
    return bitmap;
}

} // namespace Lingmo::ui

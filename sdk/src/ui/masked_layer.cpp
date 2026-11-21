// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/masked_layer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkShader.h"

namespace Lingmo::ui {

namespace {

using Pixel = std::array<float, 4>;

// Premultiplied RGBA, 0 to 255, as a GPU texture holds it.
struct Texture
{
    int width = 0;
    int height = 0;
    std::vector<Pixel> texels;

    const Pixel& at(int x, int y) const
    {
        return texels[size_t(std::clamp(y, 0, height - 1)) * size_t(width) + size_t(std::clamp(x, 0, width - 1))];
    }
};

// The shape a rounded rectangle covers, in device pixels from its corner;
// its shader's coverage without border or inset.
struct RoundedShape
{
    float width;
    float height;
    float radius;
    bool continuous;
};

} // namespace

static Pixel mix(const Pixel& a, const Pixel& b, float t)
{
    return {a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t, a[2] + (b[2] - a[2]) * t, a[3] + (b[3] - a[3]) * t};
}

// Texel space, centres at half texels, clamped to the edge.
static Pixel bilinear(const Texture& texture, float x, float y)
{
    const float fx = x - 0.5f;
    const float fy = y - 0.5f;
    const int x0 = int(std::floor(fx));
    const int y0 = int(std::floor(fy));
    const float ax = fx - float(x0);
    const float ay = fy - float(y0);
    const Pixel top = mix(texture.at(x0, y0), texture.at(x0 + 1, y0), ax);
    const Pixel bottom = mix(texture.at(x0, y0 + 1), texture.at(x0 + 1, y0 + 1), ax);
    return mix(top, bottom, ay);
}

static Pixel nearest(const Texture& texture, float x, float y)
{
    return texture.at(int(std::floor(x)), int(std::floor(y)));
}

static Pixel rounded(Pixel pixel)
{
    for (float& channel : pixel)
        channel = std::round(std::clamp(channel, 0.0f, 255.0f));
    return pixel;
}

static Texture fromImage(const SkImage& image)
{
    Texture texture;
    texture.width = image.width();
    texture.height = image.height();
    std::vector<uint8_t> bytes(size_t(texture.width) * size_t(texture.height) * 4);
    const SkImageInfo info = SkImageInfo::Make(texture.width, texture.height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    if (!image.readPixels(info, bytes.data(), size_t(texture.width) * 4, 0, 0))
        return {};
    texture.texels.resize(size_t(texture.width) * size_t(texture.height));
    for (size_t i = 0; i < texture.texels.size(); ++i)
        texture.texels[i] = {float(bytes[i * 4]), float(bytes[i * 4 + 1]), float(bytes[i * 4 + 2]), float(bytes[i * 4 + 3])};
    return texture;
}

// Each level halves the one before, sampling it at the new texels' centres,
// as the GPU's mipmap generation does.
static std::vector<Texture> mipmaps(Texture base)
{
    std::vector<Texture> levels;
    levels.push_back(std::move(base));
    while (levels.back().width > 1 || levels.back().height > 1) {
        const Texture& previous = levels.back();
        Texture next;
        next.width = std::max(1, previous.width / 2);
        next.height = std::max(1, previous.height / 2);
        next.texels.resize(size_t(next.width) * size_t(next.height));
        const float sx = float(previous.width) / float(next.width);
        const float sy = float(previous.height) / float(next.height);
        for (int y = 0; y < next.height; ++y) {
            for (int x = 0; x < next.width; ++x)
                next.texels[size_t(y) * size_t(next.width) + size_t(x)] =
                        rounded(bilinear(previous, (float(x) + 0.5f) * sx, (float(y) + 0.5f) * sy));
        }
        levels.push_back(std::move(next));
    }
    return levels;
}

static std::array<float, 2> cornerExtent(const RoundedShape& shape, float halfWidth, float halfHeight)
{
    if (shape.continuous)
        return {std::clamp(shape.radius * 1.5f, 0.0f, halfWidth), std::clamp(shape.radius * 1.5f, 0.0f, halfHeight)};
    const float r = std::clamp(shape.radius, 0.0f, std::min(halfWidth, halfHeight));
    return {r, r};
}

static float roundedDistance(const RoundedShape& shape, float px, float py)
{
    const float hx = shape.width * 0.5f;
    const float hy = shape.height * 0.5f;
    const auto [rx, ry] = cornerExtent(shape, hx, hy);
    const float ex = std::abs(px - hx) - hx;
    const float ey = std::abs(py - hy) - hy;
    const float dx = ex + rx;
    const float dy = ey + ry;
    if (std::min(rx, ry) <= 0 || std::min(dx, dy) <= 0)
        return std::max(ex, ey);
    if (!shape.continuous)
        return std::hypot(dx, dy) - rx;
    const float qx = dx / rx;
    const float qy = dy / ry;
    const float l = std::cbrt(qx * qx * qx + qy * qy * qy);
    return (l - 1) * l * l / std::hypot(qx * qx / rx, qy * qy / ry);
}

static float tangentCoverage(const RoundedShape& shape, float px, float py, float pixelSize)
{
    const float hx = shape.width * 0.5f;
    const float hy = shape.height * 0.5f;
    const auto [rx, ry] = cornerExtent(shape, hx, hy);
    const float ex = std::abs(px - hx) - hx;
    const float ey = std::abs(py - hy) - hy;
    const float cx = ex + rx;
    const float cy = ey + ry;
    float nx = ex > ey ? 1.0f : 0.0f;
    float ny = ex > ey ? 0.0f : 1.0f;
    if (std::min(rx, ry) > 0 && std::min(cx, cy) > 0) {
        nx = shape.continuous ? (cx / rx) * (cx / rx) / rx : cx;
        ny = shape.continuous ? (cy / ry) * (cy / ry) / ry : cy;
        const float length = std::hypot(nx, ny);
        nx /= length;
        ny /= length;
    }
    const float t = -roundedDistance(shape, px, py);
    const float a = std::max(std::abs(nx), std::abs(ny)) * 0.5f * pixelSize;
    const float b = std::min(std::abs(nx), std::abs(ny)) * 0.5f * pixelSize;
    if (b < 0.001f)
        return std::clamp(0.5f + t / (2 * a), 0.0f, 1.0f);
    const float u = std::abs(t);
    if (u >= a + b)
        return t > 0 ? 1.0f : 0.0f;
    if (u <= a - b)
        return 0.5f + t / (2 * a);
    const float tail = (a + b - u) * (a + b - u) / (8 * a * b);
    return t > 0 ? 1 - tail : tail;
}

static float roundedCoverage(const RoundedShape& shape, float px, float py)
{
    const float distance = roundedDistance(shape, px, py);
    if (distance < -1)
        return 1;
    if (distance > 1)
        return 0;
    const float hx = shape.width * 0.5f;
    const float hy = shape.height * 0.5f;
    const auto [rx, ry] = cornerExtent(shape, hx, hy);
    const float cx = std::abs(px - hx) - hx + rx;
    const float cy = std::abs(py - hy) - hy + ry;
    if (std::min(rx, ry) <= 0 || std::min(cx, cy) < -0.5f)
        return tangentCoverage(shape, px, py, 1);
    // Four subpixel tangents follow the curve within the pixel.
    return 0.25f * (tangentCoverage(shape, px - 0.25f, py - 0.25f, 0.5f) + tangentCoverage(shape, px + 0.25f, py - 0.25f, 0.5f)
                    + tangentCoverage(shape, px - 0.25f, py + 0.25f, 0.5f) + tangentCoverage(shape, px + 0.25f, py + 0.25f, 0.5f));
}

// A size times a scale rounds halves up.
static int roundHalfUp(double value)
{
    return int(std::floor(value + 0.5));
}

static sk_sp<SkImage> imageFromTexture(const Texture& texture)
{
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(SkImageInfo::Make(texture.width, texture.height, kRGBA_8888_SkColorType, kPremul_SkAlphaType)))
        return nullptr;
    for (int y = 0; y < texture.height; ++y) {
        auto* row = static_cast<uint8_t*>(bitmap.getAddr(0, y));
        for (int x = 0; x < texture.width; ++x) {
            const Pixel& pixel = texture.texels[size_t(y) * size_t(texture.width) + size_t(x)];
            for (int c = 0; c < 4; ++c)
                row[size_t(x) * 4 + size_t(c)] = uint8_t(pixel[size_t(c)]);
        }
    }
    bitmap.setImmutable();
    return bitmap.asImage();
}

MaskedLayerTextures renderMaskedLayerTextures(const SkImage& picture, const MaskedLayerItem& item)
{
    const double s = item.scale;
    const double w = item.width;
    const double h = item.height;
    if (w <= 0 || h <= 0 || picture.width() <= 0 || picture.height() <= 0)
        return {};

    // The image's fill mode: where the picture lands in the item, and which texels.
    const double pw = picture.width();
    const double ph = picture.height();
    double tx = 0, ty = 0, tw = w, th = h;
    double sx = 0, sy = 0, sw = pw, sh = ph;
    if (item.fillMode == MaskedLayerItem::FillMode::PreserveAspectFit) {
        if (w / pw <= h / ph) {
            tw = w;
            th = w / pw * ph;
        } else {
            tw = h / ph * pw;
            th = h;
        }
        tx = int((w - tw) / 2);
        ty = int((h - th) / 2);
    } else if (item.fillMode == MaskedLayerItem::FillMode::PreserveAspectCrop) {
        const double wscale = w / pw;
        const double hscale = h / ph;
        if (wscale > hscale) {
            const int source = int((hscale / wscale) * ph);
            sy = std::ceil((ph - source) / 2.0);
            sh = source;
        } else {
            const int source = int((wscale / hscale) * pw);
            sx = std::ceil((pw - source) / 2.0);
            sw = source;
        }
    }

    // The layer: ceil(size) * dpr texels showing the item's whole rect.
    Texture layer;
    layer.width = roundHalfUp(std::ceil(w) * s);
    layer.height = roundHalfUp(std::ceil(h) * s);
    layer.texels.assign(size_t(layer.width) * size_t(layer.height), Pixel{0, 0, 0, 0});
    const std::vector<Texture> levels = item.mipmap ? mipmaps(fromImage(picture)) : std::vector<Texture>{fromImage(picture)};
    if (levels.front().texels.empty())
        return {};
    const double texelsPerPixel = std::max(sw / (tw * layer.width / w), sh / (th * layer.height / h));
    const float lambda = item.mipmap && texelsPerPixel > 1 ? float(std::log2(texelsPerPixel)) : 0.0f;
    const int level = std::min(int(std::floor(lambda)), int(levels.size()) - 1);
    const float fraction = level + 1 < int(levels.size()) ? lambda - float(level) : 0.0f;
    const auto sampleLevel = [&](int index, double u, double v) {
        const Texture& texture = levels[size_t(index)];
        const float x = float(u * texture.width / pw);
        const float y = float(v * texture.height / ph);
        return item.smooth ? bilinear(texture, x, y) : nearest(texture, x, y);
    };
    for (int j = 0; j < layer.height; ++j) {
        const double ly = (j + 0.5) * h / layer.height;
        if (ly < ty || ly >= ty + th)
            continue;
        const double v = sy + (ly - ty) / th * sh;
        for (int i = 0; i < layer.width; ++i) {
            const double lx = (i + 0.5) * w / layer.width;
            if (lx < tx || lx >= tx + tw)
                continue;
            const double u = sx + (lx - tx) / tw * sw;
            Pixel pixel = sampleLevel(level, u, v);
            if (fraction > 0)
                pixel = mix(pixel, sampleLevel(level + 1, u, v), fraction);
            layer.texels[size_t(j) * size_t(layer.width) + size_t(i)] = rounded(pixel);
        }
    }

    // The mask: a rounded rectangle filling an item that has no parent, so
    // it snaps from the origin: whole device pixels of its size.
    const RoundedShape shape{float(std::max(std::floor(w * s + 1e-4), 1.0)),
                             float(std::max(std::floor(h * s + 1e-4), 1.0)), float(item.radius * s),
                             item.continuousCorners};
    Texture mask;
    mask.width = layer.width;
    mask.height = layer.height;
    mask.texels.assign(size_t(mask.width) * size_t(mask.height), Pixel{0, 0, 0, 0});
    for (int j = 0; j < mask.height; ++j) {
        const double ly = (j + 0.5) * h / mask.height;
        if (ly * s >= shape.height)
            continue;
        for (int i = 0; i < mask.width; ++i) {
            const double lx = (i + 0.5) * w / mask.width;
            if (lx * s >= shape.width)
                continue;
            const float coverage = std::round(roundedCoverage(shape, float(lx * s), float(ly * s)) * 255.0f);
            mask.texels[size_t(j) * size_t(mask.width) + size_t(i)] = {coverage, coverage, coverage, coverage};
        }
    }
    return {imageFromTexture(layer), imageFromTexture(mask), w, h, s};
}

void drawMaskedLayer(SkCanvas* canvas, const MaskedLayerTextures& textures, double x, double y, float opacity)
{
    if (!textures.layer || !textures.mask)
        return;
    const double s = textures.scale;
    const SkRect quad = SkRect::MakeXYWH(SkScalar(x * s), SkScalar(y * s), SkScalar(textures.width * s),
                                         SkScalar(textures.height * s));
    // Texture coordinates run across the quad, as the effect's do.
    SkMatrix local = SkMatrix::Translate(quad.x(), quad.y());
    local.preScale(quad.width() / SkScalar(textures.layer->width()), quad.height() / SkScalar(textures.layer->height()));
    const SkSamplingOptions linear(SkFilterMode::kLinear);
    sk_sp<SkShader> layer = textures.layer->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, linear, &local);
    sk_sp<SkShader> mask = textures.mask->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, linear, &local);
    SkPaint paint;
    // The opacity mask: the layer times the mask's alpha.
    paint.setShader(SkShaders::Blend(SkBlendMode::kDstIn, std::move(layer), std::move(mask)));
    paint.setAlphaf(opacity);
    canvas->drawRect(quad, paint);
}

} // namespace Lingmo::ui

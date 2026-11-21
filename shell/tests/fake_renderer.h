// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_FAKE_RENDERER_H_
#define LINGMO_TESTS_FAKE_RENDERER_H_

#include <algorithm>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include "render/renderer.h"
#include "base/region.h"

namespace Lingmo::Testsupport {

// Keeps the pixels it was given so tests can assert what was uploaded.
class FakeTexture : public render::Texture
{
public:
    FakeTexture(const base::Size &size, const uint8_t *pixels, int stride, render::PixelFormat format)
        : m_size(size)
        , m_format(format)
    {
        upload(pixels, stride);
    }

    base::Size size() const override { return m_size; }

    render::PixelFormat format() const override { return m_format; }

    void upload(const uint8_t *pixels, int stride)
    {
        const int row = m_size.width() * 4;
        m_pixels.resize(static_cast<size_t>(row) * m_size.height());
        for (int y = 0; y < m_size.height(); ++y) {
            std::memcpy(m_pixels.data() + static_cast<size_t>(y) * row,
                        pixels + static_cast<size_t>(y) * stride, row);
        }
        ++uploadCount;
    }

    const std::vector<uint8_t> &pixels() const { return m_pixels; }

    int uploadCount = 0;

private:
    base::Size m_size;
    render::PixelFormat m_format;
    std::vector<uint8_t> m_pixels;
};

// Records draw calls in order so paint ordering can be asserted without GL.
class FakeRenderer : public render::Renderer
{
public:
    struct Call {
        std::string kind;
        base::Rect rect;
        render::Color color;
        render::Texture *texture = nullptr;
        float alpha = 1.0f;
        base::AffineTransform mapping;
        base::RectF target;
    };

    bool beginFrame(render::RenderTarget *) override { return true; }

    bool endFrame() override { return true; }

    void clear(const render::Color &color) override
    {
        calls.push_back(Call {"clear", base::Rect(), color, nullptr, 1.0f});
    }

    void fillRect(const base::Rect &rect, const render::Color &color) override
    {
        calls.push_back(Call {"fill", rect, color, nullptr, 1.0f});
    }

    void drawTexture(render::Texture *texture, const base::Rect &target, float alpha) override
    {
        calls.push_back(Call {"texture", target, render::Color {}, texture, alpha});
    }

    void drawTextureTransformed(render::Texture *texture, const base::RectF &target,
                                const base::AffineTransform &transform, float alpha) override
    {
        calls.push_back(Call {"transformed", target.toEnclosingRect(), render::Color {},
                           texture, alpha, transform});
        calls.back().target = target;
    }

    void drawTextureMapped(render::Texture *texture, const base::Rect &target, float alpha,
                           const base::AffineTransform &mapping) override
    {
        drawTexture(texture, target, alpha);
        calls.back().mapping = mapping;
    }

    std::unique_ptr<render::Texture> createTexture(const base::Size &size, const uint8_t *pixels,
                                                   int stride, render::PixelFormat format) override
    {
        if (size.isEmpty() || !pixels || stride < size.width() * 4 || failTextureCreation)
            return nullptr;
        ++createdTextures;
        return std::make_unique<FakeTexture>(size, pixels, stride, format);
    }

    bool updateTexture(render::Texture *texture, const uint8_t *pixels, int stride,
                       render::PixelFormat format) override
    {
        auto *fake = dynamic_cast<FakeTexture *>(texture);
        if (!fake || !pixels || fake->format() != format || stride < fake->size().width() * 4) {
            return false;
        }
        fake->upload(pixels, stride);
        return true;
    }

    // Stands in for the GPU refusing an allocation.
    bool failTextureCreation = false;

    // What the last partial update was asked to take.
    std::optional<base::Region> lastUploadDamage;

    bool updateTexture(render::Texture *texture, const uint8_t *pixels, int stride,
                       render::PixelFormat format, const base::Region &damage) override
    {
        lastUploadDamage = damage;
        return updateTexture(texture, pixels, stride, format);
    }

    using render::Renderer::createTexture;

    // A transparent image of the size asked for; the caster is not painted.
    std::unique_ptr<render::Texture> renderShadow(const base::Size &size,
                                                  const std::vector<render::ShadowLobe> &lobes,
                                                  const std::function<void()> &) override
    {
        if (size.isEmpty())
            return nullptr;
        ++renderedShadows;
        lastShadowLobes = lobes;
        const std::vector<uint8_t> pixels(static_cast<size_t>(size.width()) * size.height() * 4);
        return std::make_unique<FakeTexture>(size, pixels.data(), size.width() * 4,
                                             render::PixelFormat::Rgba8888);
    }

    base::Size viewportSize() const override { return base::Size(1920, 1080); }

    // The kinds of the recorded calls in order.
    class KindList : public std::vector<std::string>
    {
    public:
        int count(const char *kind) const { return int(std::count(begin(), end(), kind)); }
        int count(const std::string &kind) const { return count(kind.c_str()); }
    };

    KindList kinds() const
    {
        KindList result;
        for (const Call &call : calls)
            result.push_back(call.kind);
        return result;
    }

    std::vector<Call> calls;
    int createdTextures = 0;
    int renderedShadows = 0;
    std::vector<render::ShadowLobe> lastShadowLobes;
};

} // namespace Lingmo::Testsupport

#endif // LINGMO_TESTS_FAKE_RENDERER_H_

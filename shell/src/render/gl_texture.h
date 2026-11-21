// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "render/egl_context.h"
#include "render/texture.h"
#include <optional>
#include <vector>
#include "base/region.h"

namespace Lingmo::Render {

// Production textures retain their context and select it for retirement.
class GlTexture : public Texture
{
public:
    // `target` is GL_TEXTURE_2D for uploaded pixels and GL_TEXTURE_EXTERNAL_OES
    // for EGLImage imports; passed as a plain enum so this header stays free of
    // the GL headers.
    GlTexture(uint32_t name, const base::Size &size, PixelFormat format = PixelFormat::Rgba8888,
              uint32_t target = 0x0DE1 /* GL_TEXTURE_2D */,
              std::shared_ptr<EglContext> context = {});
    GlTexture(const GlTexture &) = delete;
    GlTexture &operator=(const GlTexture &) = delete;
    ~GlTexture() override;

    base::Size size() const override { return m_size; }

    PixelFormat format() const override { return m_format; }

    base::RectF contentPixels;
    bool inverted = false; // Offscreen GL render targets have a bottom-left origin.
    bool mipmapped = false;

    uint32_t name() const { return m_name; }

    uint32_t target() const { return m_target; }

    // The context the texture belongs to; null for one deleted on whatever is current.
    const std::shared_ptr<EglContext> &context() const { return m_context; }

protected:
    std::shared_ptr<EglContext> m_context;

private:
    uint32_t m_name = 0;
    base::Size m_size;
    PixelFormat m_format = PixelFormat::Rgba8888;
    uint32_t m_target = 0x0DE1;
};

class GlTextureUploader
{
public:
    explicit GlTextureUploader(std::shared_ptr<EglContext> context)
        : m_context(std::move(context))
    {
    }

    std::unique_ptr<Texture> create(const base::Size &size, const uint8_t *pixels, int stride,
                                    PixelFormat format);
    bool update(Texture *texture, const uint8_t *pixels, int stride, PixelFormat format);
    // Only `damage`, in texture pixels, is read from `pixels` and replaced.
    bool update(Texture *texture, const uint8_t *pixels, int stride, PixelFormat format,
                const base::Region &damage);

private:
    // `rect` of the buffer at `pixels` into the same place of the bound
    // texture, or, with `create`, the whole buffer as its new image.
    void uploadPixels(const base::Size &size, const base::Rect &rect, const uint8_t *pixels, int stride,
                      bool create);
    bool unpacksRows();
    std::shared_ptr<EglContext> m_context;
    std::vector<uint8_t> m_uploadScratch;
    std::optional<bool> m_unpacksRows;
};
} // namespace Lingmo::Render

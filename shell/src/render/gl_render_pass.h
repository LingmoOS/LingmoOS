// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "render/gl_texture.h"
#include "render/renderer.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "base/geometry.h"

namespace Lingmo::Render {

template <typename T> class ScopedValue
{
public:
    explicit ScopedValue(T &slot)
        : m_slot(slot)
        , m_saved(slot)
    {
    }

    ~ScopedValue() { m_slot = std::move(m_saved); }

    ScopedValue(const ScopedValue &) = delete;
    ScopedValue &operator=(const ScopedValue &) = delete;

private:
    T &m_slot;
    T m_saved;
};

struct BackdropSource {
    unsigned int framebuffer;
    base::Size size;
    base::PointF offset;
    // How the framebuffer is turned against the layout: an output's
    // transform, 0 for an offscreen pass.
    uint32_t transform = 0;

    // `size` as the layout sees it.
    base::Size oriented() const { return transform % 2 ? size.transposed() : size; }

    // `rect`, in layout device pixels, in GL window coordinates of the
    // framebuffer, bottom up.
    base::Rect framebufferRect(const base::Rect &rect) const;
};

class GlRenderPass
{
public:
    explicit GlRenderPass(std::shared_ptr<EglContext> context)
        : m_context(std::move(context))
    {
    }

    // How texture draws combine with the target. Copy replaces every covered
    // pixel, which a pass that fills a buffer of unknown previous contents
    // needs; source-over would let them show through transparent pixels.
    enum class BlendMode {
        SourceOver,
        Copy,
    };

    const base::Size &viewport() const { return m_viewport; }

    const base::PointF &offset() const { return m_offset; }

    BlendMode blendMode() const { return m_blendMode; }

    void setGeometry(const base::Size &viewport, const base::PointF &offset)
    {
        m_viewport = viewport;
        m_offset = offset;
    }

    // The framebuffers a backdrop reads, outermost first. `transform` is the
    // current pass's own.
    std::vector<BackdropSource> backdropSources(double scale, uint32_t transform) const;
    // Frees the GL objects kept between passes; the context must be current.
    void releaseResources();
    ui::Bitmap capture(Renderer &renderer, const base::Size &size, const std::function<void()> &paint);
    ui::Bitmap renderToImage(Renderer &renderer, const base::Size &size, const std::function<void()> &paint);
    bool render(Renderer &renderer, std::unique_ptr<Texture> &texture, const base::Rect &bounds,
                const std::function<void()> &paint, const base::Rect &repaint = base::Rect());
    // Mipmaps for a render() result; the next render() into it drops them.
    void generateMipmaps(Texture *texture);
    void limitMipmapLevel(Texture *texture, int level);

    // Offscreen capture of `paint` under `context`: the logical viewport and
    // scale come from the context rather than the display, so a window can be
    // painted on its own at its own origin.
    ui::Bitmap captureImage(Renderer &renderer, const OutputRenderContext &context, const base::Size &size,
                        const std::function<void()> &paint);
    // The same, read into a pixel buffer object and collected by take(); null
    // without GLES 3.
    std::shared_ptr<PendingImage> readBack(Renderer &renderer, const OutputRenderContext &context,
                                           const base::Size &size, const std::function<void()> &paint);
    // The same, ending up top-down in the DMA-BUF behind `image`: the capture
    // texture is drawn flipped into a renderbuffer bound to the image, so the
    // pixels never leave the GPU. False if the image cannot be rendered into.
    bool captureToImage(Renderer &renderer, const OutputRenderContext &context, const base::Size &size,
                        EGLImageKHR image, const std::function<void()> &paint);

private:
    std::unique_ptr<GlTexture> captureTexture(Renderer &renderer,
                                              const OutputRenderContext &context,
                                              const base::Size &size,
                                              const std::function<void()> &paint);
    int maxTextureSize();

    std::shared_ptr<EglContext> m_context;
    int m_maxTextureSize = 0;
    base::Size m_viewport;
    base::PointF m_offset;
    BlendMode m_blendMode = BlendMode::SourceOver;
    std::vector<BackdropSource> m_sources;
    // The framebuffer objects render() draws through, one per nesting depth,
    // made on first use.
    std::vector<unsigned int> m_framebuffers;
};

} // namespace Lingmo::Render

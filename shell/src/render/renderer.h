// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_RENDER_RENDERER_H_
#define LINGMO_RENDER_RENDERER_H_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>


#include "render/dmabuf.h"
#include "render/glass.h"
#include "render/output_render_context.h"
#include "render/presentation.h"
#include "render/rounded_shape.h"
#include "render/sync_fence.h"
#include "render/texture.h"
#include "base/region.h"
#include "base/transform.h"
#include "ui/bitmap.h"
#include "ui/path.h"

namespace Lingmo::Render {

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

// One Gaussian lobe of a drop shadow, in the pixels of the image it is drawn into.
struct ShadowLobe {
    float sigma = 0;
    base::Point offset; // Positive down/right.
    float opacity = 0;
};

// A drawable owned by an output. Opaque to everything except the renderer that
// created it.
class RenderTarget
{
public:
    virtual ~RenderTarget() = default;
    virtual base::Size size() const = 0;
};

// Draw geometry is logical; rasterScale maps it into the physical target.

// Pixels on their way back from the GPU.
class PendingImage
{
public:
    virtual ~PendingImage() = default;
    // Premultiplied ARGB32, rows top-down; null if the read failed. Waits for
    // the GPU only if it has not finished by then.
    virtual ui::Bitmap take() = 0;
};
class Renderer
{
public:
    virtual ~Renderer() = default;

    PresentationSubmission *presentationSubmission = nullptr;

    void recordPresentation(const std::shared_ptr<PresentationContent> &content, const base::Rect &rect)
    {
        base::Rect viewport = m_outputContext.logicalViewport;
        if (viewport.isEmpty()) {
            const base::Size logical(std::ceil(viewportSize().width() / rasterScale()),
                                std::ceil(viewportSize().height() / rasterScale()));
            viewport = base::Rect(base::Point(), logical);
        }
        if (presentationSubmission && rect.intersects(viewport))
            presentationSubmission->add(content, false);
    }

    double rasterScale() const { return m_rasterScale; }

    void setRasterScale(double scale) { m_rasterScale = scale; }

    virtual void setOutputContext(OutputRenderContext context)
    {
        m_outputContext = std::move(context);
        m_rasterScale = m_outputContext.scale;
    }

    const OutputRenderContext &outputContext() const { return m_outputContext; }

    // The logical point nearest `logical` that lies on a physical pixel of the output.
    base::PointF pixelAligned(const base::PointF &logical) const
    {
        const base::PointF outputOrigin = m_outputContext.logicalViewport.topLeft();
        const base::PointF origin = (logical - outputOrigin) * m_rasterScale;
        return outputOrigin + base::PointF(std::round(origin.x()), std::round(origin.y())) / m_rasterScale;
    }

    // The physical pixels a client body at `logical` fills whole. The client
    // antialiases an edge that falls inside a pixel, so that pixel is left out.
    base::RectF pixelsInside(const base::RectF &logical) const
    {
        constexpr double kEpsilon = 0.01;
        const base::PointF outputOrigin = m_outputContext.logicalViewport.topLeft();
        const base::PointF start = (logical.topLeft() - outputOrigin) * m_rasterScale;
        const base::PointF end = (logical.bottomRight() - outputOrigin) * m_rasterScale;
        const base::PointF first(std::ceil(start.x() - kEpsilon), std::ceil(start.y() - kEpsilon));
        const base::PointF last(std::max(first.x(), std::floor(end.x() + kEpsilon)),
                           std::max(first.y(), std::floor(end.y() + kEpsilon)));
        const base::PointF topLeft = outputOrigin + first / m_rasterScale;
        const base::PointF bottomRight = outputOrigin + last / m_rasterScale;
        return base::RectF::fromEdges(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
    }

    // Binds `target` and sets up the viewport. Every draw call must sit between
    // beginFrame() and endFrame().
    virtual bool beginFrame(RenderTarget *target) = 0;
    virtual bool endFrame() = 0;

    // `samples` are the logical regions backdrop blurs read, which a repaint
    // reaching them has to cover whole.
    virtual void prepareDamage(const base::Region &, const std::vector<base::Rect> &) { }

    // Whether this frame draws any of `logical` again. Everywhere else the
    // target keeps what the last frame left there.
    virtual bool repaints(const base::Rect &) const { return true; }

    // The logical rectangle outside which nothing drawn now reaches the target,
    // the repaint of an output frame or the bounds of a pass; none culls nothing.
    std::optional<base::Rect> paintClip;

    // False when a draw inside `logical` would be clipped away whole, so it
    // and any work feeding only it can be skipped, as viz's ShouldSkipQuad.
    bool drawsInto(const base::Rect &logical) const
    {
        return !paintClip || paintClip->intersects(logical);
    }

    // Ends the frame without presenting it. Used when the display controller
    // decided to scan a client buffer out instead, so the previous composited
    // buffer must be left untouched and reusable.
    virtual void cancelFrame() { }

    // Animation snapshots retain the body; the physical-width outline is drawn afterward.
    bool suppressWindowOutlines = false;

    // A window drawn scaled from its snapshot is not where its blurs would
    // read the screen, so it goes without them.
    bool suppressBackgroundBlur = false;

    // A window drawn by the desk bar while it is on another desk:
    // its surfaces hidden along with it are drawn too.
    bool revealHiddenSurfaces = false;

    // Multiplies the opacity of every surface and colour drawn: the overview
    // darkens the desktop as it draws it rather than over it, one full-output
    // pass fewer for an iGPU that composes a frame in several milliseconds.
    float opacityScale = 1;
    // With it, an opaque texture drawn at an opacity below 1 replaces the
    // pixels under it, scaled, instead of blending over them: the overview
    // darkens a desktop that covers the output without clearing first.
    bool overwriteOpaque = false;

    // Set by an animation morphing a window's corners into its snapshot: the
    // window's visual node clips its content with these instead of its own.
    struct WindowCorners {
        double radius = 0;
        base::SizeF radiusScale {1, 1};
    };
    std::optional<WindowCorners> windowCorners;
    bool capturing = false;

    // The output's presentation-timeline time for the frame being composited,
    // in milliseconds of the steady clock; -1 outside an output frame. What
    // Chromium animations step on as BeginFrameArgs::frame_time. Fractional:
    // whole milliseconds would step a 13.3 ms period by 13 or 14 at random.
    double frameTime = -1;
    // The output's refresh interval in milliseconds; 0 outside an output frame.
    double framePeriod = 0;

    virtual void pushRoundedClip(const RoundedShape &shape) { m_roundedClips.push_back(shape); }

    virtual void popRoundedClip()
    {
        if (!m_roundedClips.empty())
            m_roundedClips.pop_back();
    }

    virtual void drawRoundedOutline(const RoundedShape &, const Color &, double) { }

    virtual void clear(const Color &color) = 0;
    virtual void fillRect(const base::Rect &rect, const Color &color) = 0;

    virtual void fillRectF(const base::RectF &rect, const Color &color)
    {
        fillRect(rect.toEnclosingRect(), color);
    }

    virtual void drawTextureF(Texture *texture, const base::RectF &target, float alpha = 1.0f)
    {
        drawTexture(texture, target.toEnclosingRect(), alpha);
    }

    virtual void drawTexture(Texture *texture, const base::Rect &target, float alpha = 1.0f) = 0;

    // `transform` maps logical coordinates, so `target` can be rotated or scaled.
    virtual void drawTextureTransformed(Texture *texture, const base::RectF &target,
                                        const base::AffineTransform &transform, float alpha = 1.0f)
    {
        drawTextureF(texture, transform.mapRect(target), alpha);
    }

    virtual void drawTextureMapped(Texture *texture, const base::Rect &target, float alpha,
                                   const base::AffineTransform &mapping) = 0;

    struct MappedRect {
        base::RectF target;
        base::AffineTransform mapping;
    };

    // drawTextureMapped() for several rectangles of one texture, as the
    // slices of a shadow: a renderer that can draws them in one call.
    virtual void drawTextureMappedRects(Texture *texture, const std::vector<MappedRect> &rects,
                                        float alpha)
    {
        for (const MappedRect &rect : rects)
            drawTextureMapped(texture, rect.target.toEnclosingRect(), alpha, rect.mapping);
    }

    // A non-empty `shape`, in the same coordinates as `bounds`, masks the blur
    // instead of the rounded rectangle. In a BackdropPass, the blur of surface
    // `owner`, by damage id, keeps its result for the next pass.
    virtual void drawBackgroundBlur(const base::Rect &bounds, int radius, const base::Rect &clip,
                                    const GlassParams &glass = {}, const ui::Path &shape = {},
                                    uint64_t owner = 0)
    {
    }

    // Blur results kept between passes into the target `scope` names.
    virtual bool keepsBackdrop(uint64_t /*scope*/, uint64_t /*owner*/) const { return false; }
    virtual void beginBackdropPass(uint64_t /*scope*/, const std::vector<uint64_t> & /*fresh*/) { }
    // Zero outside a BackdropPass, and in the passes nested in one.
    uint64_t backdropScope = 0;

    // Makes subsequent draws wait for `fence` before sampling. Renderers that
    // can queue the wait on the GPU do so; the base implementation blocks,
    // which is the correct fallback and never wrong, only slower.
    virtual void waitFence(const SyncFence &fence) { fence.wait(kFenceWaitMs); }

    // Signalled once every draw issued so far has finished reading its sources.
    // An invalid fence means the renderer cannot produce one, in which case the
    // caller must fall back to releasing buffers on frame completion.
    virtual SyncFence createReleaseFence() { return SyncFence(); }

    // A wedged client must not stall the compositor forever.
    static constexpr int kFenceWaitMs = 500;

    // Uploads `size` pixels starting at `pixels`, `stride` bytes per row.
    virtual std::unique_ptr<Texture> createTexture(const base::Size &size, const uint8_t *pixels,
                                                   int stride, PixelFormat format) = 0;

    // Re-uploads into an existing texture. Returns false when `texture` was not
    // created by this renderer, cannot be uploaded into, or its format or the
    // row stride no longer fit it, in which case the caller must create a new
    // one. The size is the texture's own; a caller whose pixels changed size
    // compares that itself.
    virtual bool updateTexture(Texture *texture, const uint8_t *pixels, int stride,
                               PixelFormat format) = 0;

    // The same for `damage` alone, in buffer pixels; everywhere else the
    // texture keeps what it holds. A renderer without partial uploads takes
    // the whole buffer, which outside `damage` holds the same pixels.
    virtual bool updateTexture(Texture *texture, const uint8_t *pixels, int stride,
                               PixelFormat format, const base::Region &)
    {
        return updateTexture(texture, pixels, stride, format);
    }

    // Tightly packed RGBA8888, `size.width() * size.height() * 4` bytes.
    std::unique_ptr<Texture> createTexture(const base::Size &size, const uint8_t *rgba)
    {
        return createTexture(size, rgba, size.width() * 4, PixelFormat::Rgba8888);
    }

    // Optional offscreen composition, called only inside a frame. The resulting
    // premultiplied texture owns its pixels independently of client resources.
    virtual bool renderToTexture(std::unique_ptr<Texture> &texture, const base::Rect &bounds,
                                 const std::function<void()> &paint)
    {
        return false;
    }

    // Draws only `repaint` again, when `texture` holds the rest from an
    // earlier pass over the same bounds at the same scale; an empty `repaint`
    // or a texture of another size draws everything.
    virtual bool renderToTexture(std::unique_ptr<Texture> &texture, const base::Rect &bounds,
                                 const base::Rect &repaint, const std::function<void()> &paint)
    {
        return renderToTexture(texture, bounds, paint);
    }

    // A `size` pixel image of black `lobes` cast by what `paintCaster` draws in
    // opaque black, with the caster's own coverage cut back out. The caster is
    // painted with no clip, one logical unit to an image pixel. Only inside a
    // frame; null where the renderer cannot draw one.
    virtual std::unique_ptr<Texture> renderShadow(const base::Size &size,
                                                  const std::vector<ShadowLobe> &lobes,
                                                  const std::function<void()> &paintCaster)
    {
        return nullptr;
    }

    // Samples a renderToTexture() result through mipmaps until the next
    // renderToTexture() into it, for a snapshot drawn far below its size.
    // cc's EffectNode::trilinear_filtering, a render pass with generate_mipmap.
    virtual void enableTrilinearFiltering(Texture *texture) { }

    // Samples a mipmapped texture from `level` or a larger one, never a
    // smaller; a negative level lifts the limit.
    virtual void limitMipmapLevel(Texture *texture, int level) { }

    // Capture only inside a frame; the display render target remains untouched.
    virtual ui::Bitmap captureFrame(const std::function<void()> &paint) { return {}; }

    // Capture of `paint` under `context` rather than the display's: a window
    // is painted on its own by making its bounds the logical viewport. Only
    // inside a frame; premultiplied ARGB32, rows top-down.
    virtual ui::Bitmap captureImage(const OutputRenderContext &context, const base::Size &size,
                                    const std::function<void()> &paint)
    {
        return {};
    }

    // captureFrame() and captureImage() without waiting for the GPU: the
    // pixels are read into a buffer and collected later, as viz serves a copy
    // request with an asynchronous readback. Null where only the waiting path
    // exists.
    virtual std::shared_ptr<PendingImage> captureFrameLater(const std::function<void()> &paint)
    {
        return nullptr;
    }

    virtual std::shared_ptr<PendingImage> captureImageLater(const OutputRenderContext &context,
                                                            const base::Size &size,
                                                            const std::function<void()> &paint)
    {
        return nullptr;
    }

    // The same capture, rendered on the GPU into a DMA-BUF the client owns.
    // `attributes` must outlive the call. False when the buffer cannot be a
    // render target here; captureDmaBufFormats() says what can.
    virtual bool captureToDmaBuf(const OutputRenderContext &context, const base::Size &size,
                                 const DmaBufAttributes &attributes,
                                 const std::function<void()> &paint)
    {
        return false;
    }

    // Formats and modifiers captureToDmaBuf() accepts, empty without support,
    // and the render node clients should allocate such buffers on (0: unknown).
    virtual const DmaBufFormats &captureDmaBufFormats() const
    {
        static const DmaBufFormats none;
        return none;
    }

    virtual dev_t captureDmaBufDevice() const { return 0; }

    // Inside a capture of the output this frame draws: copies what the frame
    // composited, `size` pixels, in place of painting it again, as viz serves
    // a copy request from the root render pass. False when it cannot.
    virtual bool copyOutputFrame(const base::Size &size) { return false; }

    // Reads back what `paint` draws into a `size` pixel image whose logical
    // coordinates are its pixels. Only inside a frame; premultiplied ARGB32.
    virtual ui::Bitmap renderToImage(const base::Size &size, const std::function<void()> &paint)
    {
        return {};
    }

    virtual base::Size viewportSize() const = 0;

    // Small immutable raster assets, owned by this renderer. Call only with its
    // context current; returned references last until the next cache lookup.
    using TextureSet = std::vector<std::unique_ptr<Texture>>;
    const TextureSet &cachedTextureSet(const std::string &key, const std::function<TextureSet()> &make);

    // Derived GPU renderers clear these before destroying their context.
    void clearTextureCache() { m_textureCache.clear(); }

    size_t textureCacheSize() const { return m_textureCache.size(); }

    uint64_t textureCacheMisses() const { return m_textureCacheMisses; }

    static constexpr size_t textureCacheLimit = 16;

protected:
    std::vector<RoundedShape> m_roundedClips;

private:
    double m_rasterScale = 1;
    OutputRenderContext m_outputContext;

    struct CachedTextures {
        std::string key;
        TextureSet textures;
    };

    std::list<CachedTextures> m_textureCache;
    uint64_t m_textureCacheMisses = 0;
};

// A pass into the target `scope` names whose blurs keep their results for
// the next: those of `fresh`, whose backdrop has not changed since the last
// pass, draw what they kept, and the others' results are dropped.
class BackdropPass
{
public:
    BackdropPass(Renderer &renderer, uint64_t scope, const std::vector<uint64_t> &fresh)
        : m_renderer(renderer)
        , m_saved(std::exchange(renderer.backdropScope, scope))
    {
        renderer.beginBackdropPass(scope, fresh);
    }

    ~BackdropPass() { m_renderer.backdropScope = m_saved; }

    BackdropPass(const BackdropPass &) = delete;
    BackdropPass &operator=(const BackdropPass &) = delete;

private:
    Renderer &m_renderer;
    uint64_t m_saved;
};

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_RENDERER_H_

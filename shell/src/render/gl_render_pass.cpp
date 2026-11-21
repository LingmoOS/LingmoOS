// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/gl_render_pass.h"

#include <GLES3/gl3.h>
// Extension declarations require the core GL types first.
#include <GLES2/gl2ext.h>
#include <cmath>
#include <cstring>
#include <optional>
#include <utility>

#include "render/gl_program.h"
#include <cassert>
#include "base/geometry.h"

namespace Lingmo::Render {
namespace {

// The target an offscreen pass draws over and gives back: the framebuffer,
// the viewport and the scissor. Every draw here sets its own program,
// textures, blend and vertex attributes, so none of that is saved, as
// gl::ScopedFramebufferBinder restores only the binding it took and Skia's
// GrGLGpu re-flushes its cached state per draw instead of reading it back.
// A pixel transfer buffer left bound would turn the pass's own uploads and
// readbacks into buffer offsets, so those two bindings are dropped.
class ScopedGlState
{
public:
    explicit ScopedGlState(const EglContext &context)
        : es3(context.glVersion().starts_with("OpenGL ES 3"))
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
        if (es3) {
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);
        scissored = glIsEnabled(GL_SCISSOR_TEST);
        glDisable(GL_SCISSOR_TEST);
    }

    ~ScopedGlState()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        if (es3)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        scissored ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
    }

    ScopedGlState(const ScopedGlState &) = delete;
    ScopedGlState &operator=(const ScopedGlState &) = delete;

private:
    bool es3;
    GLint framebuffer = 0, readFramebuffer = 0, viewport[4] = {}, scissor[4] = {};
    bool scissored = false;
};

class Framebuffer
{
public:
    explicit Framebuffer(GLuint texture)
    {
        glGenFramebuffers(1, &m_name);
        glBindFramebuffer(GL_FRAMEBUFFER, m_name);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    }

    ~Framebuffer() { glDeleteFramebuffers(1, &m_name); }

    Framebuffer(const Framebuffer &) = delete;
    Framebuffer &operator=(const Framebuffer &) = delete;

    bool complete() const
    {
        return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

private:
    GLuint m_name = 0;
};

class RendererState
{
public:
    explicit RendererState(Renderer &renderer)
        : m_renderer(renderer)
        , m_context(renderer.outputContext())
        , m_scale(renderer.rasterScale())
        , m_submission(renderer.presentationSubmission)
        , m_capturing(renderer.capturing)
        , m_backdropScope(std::exchange(renderer.backdropScope, 0))
        , m_paintClip(std::exchange(renderer.paintClip, std::nullopt))
    {
        renderer.presentationSubmission = nullptr;
    }

    ~RendererState()
    {
        m_renderer.Renderer::setOutputContext(m_context);
        m_renderer.setRasterScale(m_scale);
        m_renderer.presentationSubmission = m_submission;
        m_renderer.capturing = m_capturing;
        m_renderer.backdropScope = m_backdropScope;
        m_renderer.paintClip = m_paintClip;
    }

    RendererState(const RendererState &) = delete;
    RendererState &operator=(const RendererState &) = delete;

private:
    Renderer &m_renderer;
    OutputRenderContext m_context;
    double m_scale;
    PresentationSubmission *m_submission;
    bool m_capturing;
    uint64_t m_backdropScope;
    std::optional<base::Rect> m_paintClip;
};

std::unique_ptr<GlTexture> allocate(const std::shared_ptr<EglContext> &context, const base::Size &size,
                                    GLint filter)
{
    GLuint name = 0;
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    return std::make_unique<GlTexture>(name, size, PixelFormat::Rgba8888, GL_TEXTURE_2D, context);
}

class GlPendingImage : public PendingImage
{
public:
    GlPendingImage(std::shared_ptr<EglContext> context, GLuint buffer, const base::Size &size)
        : m_context(std::move(context))
        , m_buffer(buffer)
        , m_size(size)
    {
    }

    ~GlPendingImage() override
    {
        ScopedEglCurrent current(*m_context);
        if (current.valid())
            glDeleteBuffers(1, &m_buffer);
    }

    ui::Bitmap take() override
    {
        // Output sessions of one output share the read.
        if (m_taken)
            return m_image;
        m_taken = true;
        ScopedEglCurrent current(*m_context);
        if (!current.valid())
            return {};
        GLint previous = 0;
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &previous);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, m_buffer);
        const size_t bytes = size_t(m_size.width()) * m_size.height() * 4;
        ui::Bitmap image(m_size, ui::Bitmap::Format::Rgba8888Premultiplied);
        const void *pixels = image.isNull()
                ? nullptr
                : glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, bytes, GL_MAP_READ_BIT);
        if (pixels) {
            std::memcpy(image.bits(), pixels, bytes);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, previous);
        if (pixels)
            m_image = image.mirrored(false, true).converted(ui::Bitmap::Format::Argb32Premultiplied);
        return m_image;
    }

private:
    std::shared_ptr<EglContext> m_context;
    GLuint m_buffer;
    base::Size m_size;
    bool m_taken = false;
    ui::Bitmap m_image;
};

} // namespace

base::Rect BackdropSource::framebufferRect(const base::Rect &rect) const
{
    const base::Size layout = oriented();
    const base::Rect r = rect.translated(-offset.toPoint());
    base::Rect turned = r;
    switch (transform) {
    case 1:
        turned = base::Rect(r.y(), layout.width() - r.x() - r.width(), r.height(), r.width());
        break;
    case 2:
        turned = base::Rect(layout.width() - r.x() - r.width(), layout.height() - r.y() - r.height(),
                       r.width(), r.height());
        break;
    case 3:
        turned = base::Rect(layout.height() - r.y() - r.height(), r.x(), r.height(), r.width());
        break;
    }
    return base::Rect(turned.x(), size.height() - turned.y() - turned.height(), turned.width(),
                 turned.height());
}

std::vector<BackdropSource> GlRenderPass::backdropSources(double scale, uint32_t transform) const
{
    auto sources = m_sources;
    GLint framebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
    sources.push_back({static_cast<unsigned int>(framebuffer), m_viewport, m_offset, transform});
    for (auto &source : sources)
        source.offset *= scale;
    return sources;
}

ui::Bitmap GlRenderPass::capture(Renderer &renderer, const base::Size &size,
                                 const std::function<void()> &paint)
{
    return captureImage(renderer, renderer.outputContext(), size, paint);
}

std::unique_ptr<GlTexture> GlRenderPass::captureTexture(Renderer &renderer,
                                                        const OutputRenderContext &context,
                                                        const base::Size &size,
                                                        const std::function<void()> &paint)
{
    if (size.isEmpty())
        return nullptr;
    const GLint limit = maxTextureSize();
    if (size.width() > limit || size.height() > limit)
        return nullptr;
    auto texture = allocate(m_context, size, GL_NEAREST);
    Framebuffer framebuffer(texture->name());
    if (!framebuffer.complete())
        return nullptr;
    ScopedValue viewport(m_viewport);
    ScopedValue offset(m_offset);
    renderer.Renderer::setOutputContext(context);
    // Letterboxed content keeps its place in the buffer, as on the display.
    const base::Rect content = context.nativeContentRect.isEmpty() ? base::Rect(base::Point(), size)
                                                              : context.nativeContentRect;
    m_viewport = content.size();
    m_offset = context.logicalViewport.topLeft();
    glViewport(content.x(), size.height() - content.y() - content.height(), content.width(),
               content.height());
    glDisable(GL_SCISSOR_TEST);
    renderer.capturing = true;
    renderer.clear({0, 0, 0, 0});
    paint();
    // Rows are bottom-up, as with every render into a texture here.
    texture->inverted = true;
    return texture;
}

ui::Bitmap GlRenderPass::captureImage(Renderer &renderer, const OutputRenderContext &context,
                                      const base::Size &size, const std::function<void()> &paint)
{
    ScopedGlState state(*m_context);
    RendererState rendererState(renderer);
    ui::Bitmap image(size, ui::Bitmap::Format::Rgba8888Premultiplied);
    if (image.isNull())
        return {};
    auto texture = captureTexture(renderer, context, size, paint);
    if (!texture)
        return {};
    Framebuffer framebuffer(texture->name());
    if (!framebuffer.complete())
        return {};
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    if (glGetError() != GL_NO_ERROR)
        return {};
    return image.mirrored(false, true).converted(ui::Bitmap::Format::Argb32Premultiplied);
}

std::shared_ptr<PendingImage> GlRenderPass::readBack(Renderer &renderer,
                                                     const OutputRenderContext &context,
                                                     const base::Size &size,
                                                     const std::function<void()> &paint)
{
    if (!m_context->glVersion().starts_with("OpenGL ES 3"))
        return nullptr;
    ScopedGlState state(*m_context);
    RendererState rendererState(renderer);
    auto texture = captureTexture(renderer, context, size, paint);
    if (!texture)
        return nullptr;
    Framebuffer framebuffer(texture->name());
    if (!framebuffer.complete())
        return nullptr;
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    auto pending = std::make_shared<GlPendingImage>(m_context, buffer, size);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_PACK_BUFFER, GLsizeiptr(size.width()) * size.height() * 4, nullptr,
                 GL_STREAM_READ);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    // Into the buffer: returns once queued, not once the GPU is done.
    glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    if (glGetError() != GL_NO_ERROR)
        return nullptr;
    return pending;
}

// The capture texture is drawn flipped into a texture bound to the DMA-BUF's
// EGLImage, the way viz copies a drawn render pass out with a flipped scaler
// pass instead of reading pixels back (GLRenderer::CopyDrawnRenderPass), and
// the image is bound as a plain GL_TEXTURE_2D render target as ui::
// NativePixmapEGLBinding does for RGB pixmaps.
bool GlRenderPass::captureToImage(Renderer &renderer, const OutputRenderContext &context,
                                  const base::Size &size, EGLImageKHR image,
                                  const std::function<void()> &paint)
{
    ScopedGlState state(*m_context);
    RendererState rendererState(renderer);
    auto imageTargetTexture = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
            eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!imageTargetTexture || image == EGL_NO_IMAGE_KHR)
        return false;
    auto texture = captureTexture(renderer, context, size, paint);
    if (!texture)
        return false;

    GLuint name = 0;
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    imageTargetTexture(GL_TEXTURE_2D, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    bool complete = glGetError() == GL_NO_ERROR;
    if (complete) {
        Framebuffer framebuffer(name);
        complete = framebuffer.complete();
        if (complete) {
            // A plain identity pass. Sampling the bottom-up capture as if it
            // were top-down lands it top-down in the target's memory.
            OutputRenderContext flat;
            flat.outputId = context.outputId;
            flat.logicalViewport = base::Rect(base::Point(), size);
            flat.nativeTargetSize = size;
            flat.nativeContentRect = base::Rect(base::Point(), size);
            ScopedValue viewport(m_viewport);
            ScopedValue offset(m_offset);
            ScopedValue blend(m_blendMode);
            renderer.Renderer::setOutputContext(flat);
            m_viewport = size;
            m_offset = base::PointF();
            // The client reuses its buffers, so every pixel has to be written.
            m_blendMode = BlendMode::Copy;
            glViewport(0, 0, size.width(), size.height());
            glDisable(GL_SCISSOR_TEST);
            texture->inverted = false;
            renderer.drawTexture(texture.get(), base::Rect(base::Point(), size));
            glFlush();
        }
    }
    glDeleteTextures(1, &name);
    return complete;
}

ui::Bitmap GlRenderPass::renderToImage(Renderer &renderer, const base::Size &size,
                                       const std::function<void()> &paint)
{
    RendererState rendererState(renderer);
    if (size.isEmpty())
        return {};
    ScopedGlState state(*m_context);
    const GLint limit = maxTextureSize();
    if (size.width() > limit || size.height() > limit)
        return {};
    ui::Bitmap image(size, ui::Bitmap::Format::Rgba8888Premultiplied);
    if (image.isNull())
        return {};
    auto texture = allocate(m_context, size, GL_NEAREST);
    GLint previous = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous);
    Framebuffer framebuffer(texture->name());
    if (!framebuffer.complete())
        return {};
    ScopedValue sources(m_sources);
    ScopedValue viewport(m_viewport);
    ScopedValue offset(m_offset);
    m_sources.push_back({static_cast<unsigned int>(previous), m_viewport, m_offset,
                         renderer.outputContext().transform});
    auto context = renderer.outputContext();
    context.transform = 0;
    context.scale = 1;
    context.logicalViewport = base::Rect(base::Point(), size);
    context.nativeContentRect = base::Rect(base::Point(), size);
    renderer.Renderer::setOutputContext(context);
    m_viewport = size;
    m_offset = base::PointF();
    glViewport(0, 0, size.width(), size.height());
    glDisable(GL_SCISSOR_TEST);
    renderer.clear({0, 0, 0, 0});
    paint();
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    if (glGetError() != GL_NO_ERROR)
        return {};
    return image.mirrored(false, true).converted(ui::Bitmap::Format::Argb32Premultiplied);
}

bool GlRenderPass::render(Renderer &renderer, std::unique_ptr<Texture> &texture,
                          const base::Rect &bounds, const std::function<void()> &paint,
                          const base::Rect &repaint)
{
    RendererState rendererState(renderer);
    if (bounds.isEmpty())
        return false;
    ScopedGlState state(*m_context);
    // Device pixels counted from the viewport's origin, the grid pixelAligned()
    // puts content on: a window captured on its own has a viewport whose origin
    // falls inside a pixel at a fractional scale.
    const base::PointF origin = renderer.outputContext().logicalViewport.topLeft();
    const base::RectF deviceBounds((base::PointF(bounds.topLeft()) - origin) * renderer.rasterScale(),
                              base::SizeF(bounds.size()) * renderer.rasterScale());
    const base::Rect coverage = deviceBounds.toEnclosingRect();
    // Rounded outward, the coverage is a pixel wider or taller depending on
    // where its fractional origin falls, so at a fractional scale a moving
    // window would take a new texture nearly every frame. The texture is sized
    // for the largest coverage the bounds can round out to and keeps its size
    // as the origin moves; contentPixels says where the content is in it, and
    // the spare row and column stay clear.
    const bool fractional = renderer.rasterScale() != std::floor(renderer.rasterScale());
    const base::Size allocation(int(std::ceil(bounds.width() * renderer.rasterScale())) + fractional,
                           int(std::ceil(bounds.height() * renderer.rasterScale())) + fractional);
    assert(allocation.width() >= coverage.width() && allocation.height() >= coverage.height());
    const GLint limit = maxTextureSize();
    if (allocation.width() > limit || allocation.height() > limit)
        return false;
    auto *target = dynamic_cast<GlTexture *>(texture.get());
    // Only a texture of this size still holds what lies outside `repaint`.
    base::Rect scissor;
    if (!target || target->size() != allocation) {
        texture = allocate(m_context, allocation, GL_LINEAR);
        target = static_cast<GlTexture *>(texture.get());
        target->inverted = true;
    } else if (!repaint.isEmpty()) {
        scissor = base::RectF((base::PointF(repaint.topLeft()) - origin) * renderer.rasterScale(),
                         base::SizeF(repaint.size()) * renderer.rasterScale())
                          .toEnclosingRect()
                          .translated(-coverage.topLeft())
                          .intersected(base::Rect(base::Point(), allocation));
    }
    if (target->mipmapped) {
        // The smaller levels would keep showing the previous contents.
        glBindTexture(GL_TEXTURE_2D, target->name());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        target->mipmapped = false;
    }
    target->contentPixels = deviceBounds.translated(-coverage.topLeft());
    GLint previous = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous);
    // Every window and blur pass runs through here each frame, so framebuffer
    // objects are kept and re-pointed rather than created and destroyed per
    // pass: one per nesting depth, since a blur inside a window's pass copies
    // out of the framebuffer the window is being drawn into.
    const size_t depth = m_sources.size();
    while (m_framebuffers.size() <= depth) {
        GLuint name = 0;
        glGenFramebuffers(1, &name);
        m_framebuffers.push_back(name);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[depth]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->name(), 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    ScopedValue sources(m_sources);
    ScopedValue viewport(m_viewport);
    ScopedValue offset(m_offset);
    m_sources.push_back({static_cast<unsigned int>(previous), m_viewport, m_offset,
                         renderer.outputContext().transform});
    auto context = renderer.outputContext();
    context.transform = 0;
    context.scale = renderer.rasterScale();
    renderer.Renderer::setOutputContext(context);
    m_viewport = allocation;
    m_offset = origin + base::PointF(coverage.topLeft()) / renderer.rasterScale();
    glViewport(0, 0, allocation.width(), allocation.height());
    if (scissor.isEmpty()) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        // Rows are bottom-up in the texture.
        glEnable(GL_SCISSOR_TEST);
        glScissor(scissor.x(), allocation.height() - scissor.y() - scissor.height(),
                  scissor.width(), scissor.height());
    }
    // The scissor rounds out at a fractional scale; a pixel more keeps what reaches its edge.
    renderer.paintClip = scissor.isEmpty() ? bounds
                                           : bounds.intersected(repaint.adjusted(-1, -1, 1, 1));
    renderer.clear({0, 0, 0, 0});
    paint();
    return true;
}

void GlRenderPass::releaseResources()
{
    if (!m_framebuffers.empty())
        glDeleteFramebuffers(GLsizei(m_framebuffers.size()), m_framebuffers.data());
    m_framebuffers.clear();
}

int GlRenderPass::maxTextureSize()
{
    if (!m_maxTextureSize) {
        GLint limit = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
        m_maxTextureSize = limit;
    }
    return m_maxTextureSize;
}

void GlRenderPass::generateMipmaps(Texture *texture)
{
    auto *target = dynamic_cast<GlTexture *>(texture);
    if (!target || target->mipmapped || target->target() != GL_TEXTURE_2D)
        return;
    // GLES 2 only mipmaps power-of-two sizes.
    if (!m_context->glVersion().starts_with("OpenGL ES 3")
        && !m_context->supportsGl("GL_OES_texture_npot"))
        return;
    glBindTexture(GL_TEXTURE_2D, target->name());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    target->mipmapped = glGetError() == GL_NO_ERROR;
    if (!target->mipmapped)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GlRenderPass::limitMipmapLevel(Texture *texture, int level)
{
    auto *target = dynamic_cast<GlTexture *>(texture);
    // GLES 2 has no LOD range.
    if (!target || !target->mipmapped || !m_context->glVersion().starts_with("OpenGL ES 3"))
        return;
    glBindTexture(GL_TEXTURE_2D, target->name());
    // 1000 is GL's initial GL_TEXTURE_MAX_LOD.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, level < 0 ? 1000.0f : float(level));
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Lingmo::Render

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/egl_renderer.h"

#include <GLES3/gl3.h>
// Extension declarations require the core GL types first.
#include <GLES2/gl2ext.h>
#include <algorithm>
#include <cmath>
#include <drm_fourcc.h>
#include <gbm.h>
#include <sys/stat.h>
#include <xf86drm.h>

#include "core/logging.h"
#include "render_shaders.h"
#include <cassert>
#include "base/region.h"
#include "base/transform.h"

namespace Lingmo::Render {

EglRenderer::EglRenderer(std::shared_ptr<EglContext> context, gbm_device *device)
    : m_context(std::move(context))
    , m_fences(*m_context)
    , m_timer(*m_context)
    , m_importer(m_context, device)
    , m_upload(m_context)
    , m_pass(m_context)
    , m_device(device)
{
}

EglRenderer::~EglRenderer()
{
    if (!m_colorProgram && !m_textureProgram && !m_externalProgram && !m_backdrop
        && !m_dropShadow && !textureCacheSize())
        return;
    ScopedEglCurrent current(*m_context);
    if (!current.valid())
        LM_FATAL(lingmoRender) << "Cannot make EGL context current for renderer retirement";
    m_backdrop.reset();
    m_dropShadow.reset();
    m_pass.releaseResources();
    clearTextureCache();
    m_colorProgram.reset();
    m_textureProgram.reset();
    m_externalProgram.reset();
}

std::unique_ptr<EglRenderer> EglRenderer::create(gbm_device *device, uint32_t format)
{
    auto context = EglContext::create(device, format);
    if (!context)
        return nullptr;
    return std::unique_ptr<EglRenderer>(new EglRenderer(std::move(context), device));
}

std::unique_ptr<EglRenderTarget> EglRenderer::createTarget(gbm_surface *surface, const base::Size &size)
{
    auto target = EglRenderTarget::create(m_context->display(), m_context->config(), surface, size);
    if (!target || !m_context->makeCurrent(target->eglSurface()))
        return nullptr;
    if (!m_colorProgram && !initPrograms()) {
        m_context->makeCurrent(m_lastTarget ? m_lastTarget->eglSurface() : EGL_NO_SURFACE);
        return nullptr;
    }
    m_lastTarget = target.get();
    eglSwapInterval(m_context->display(), 1);
    return target;
}

bool EglRenderer::initPrograms(const char *fragmentOverride, const char *externalOverride,
                               const char *backdropOverride)
{
    auto color =
            GlProgram::create(shaders::color_vertex_shader,
                              (fragmentOverride ? std::string(fragmentOverride)
                                                : clippedFragment(shaders::color_fragment_shader))
                                      .c_str());
    auto texture = GlProgram::create(shaders::texture_vertex_shader,
                                     clippedFragment(shaders::texture_fragment_shader).c_str());
    if (!color || !texture)
        return false;
    auto external =
            GlProgram::create(shaders::texture_vertex_shader,
                              (externalOverride ? std::string(externalOverride)
                                                : clippedFragment(shaders::external_fragment_shader,
                                                                  shaders::external_extension))
                                      .c_str());
    if (!external)
        LM_WARNING(lingmoRender)
                << "GL_OES_EGL_image_external is unavailable; DMA-BUF buffers cannot be composited";
    // Blur and glass are effects: a driver that cannot build them still gets a
    // desktop, drawn without them.
    auto backdrop = std::make_unique<BackdropFilter>(m_context);
    if (backdrop->initialize(backdropOverride))
        m_backdrop = std::move(backdrop);
    else
        LM_WARNING(lingmoRender) << "Blur and glass shaders are unavailable; drawing without them";
    auto dropShadow = std::make_unique<DropShadow>();
    if (dropShadow->initialize())
        m_dropShadow = std::move(dropShadow);
    else
        LM_WARNING(lingmoRender) << "The shadow shader is unavailable; drawing without shadows";
    m_colorProgram = std::move(color);
    m_textureProgram = std::move(texture);
    m_externalProgram = std::move(external);
    // A deleted program's name can come back on a new one.
    m_clipUniforms.clear();
    m_colorUniform = m_colorProgram.uniform("uColor");
    resolveTextureUniforms(m_textureUniforms, m_textureProgram);
    if (m_externalProgram)
        resolveTextureUniforms(m_externalUniforms, m_externalProgram);
    return true;
}

void EglRenderer::resolveTextureUniforms(TextureUniforms &uniforms, const GlProgram &program)
{
    uniforms = TextureUniforms();
    uniforms.texture = program.uniform("uTexture");
    uniforms.alpha = program.uniform("uAlpha");
    uniforms.swizzle = program.uniform("uSwizzle");
    uniforms.hasAlpha = program.uniform("uHasAlpha");
    // The sampler always reads unit 0.
    glUseProgram(program);
    glUniform1i(uniforms.texture, 0);
}

EglRenderer::ClipUniforms &EglRenderer::clipUniforms(const GlProgram &program)
{
    for (ClipUniforms &entry : m_clipUniforms) {
        if (entry.program == program)
            return entry;
    }
    ClipUniforms entry;
    entry.program = program;
    entry.count = program.uniform("uClipCount");
    entry.clips = program.uniform("uClips");
    entry.clipRadii = program.uniform("uClipRadii");
    entry.viewportRect = program.uniform("uViewportRect");
    entry.outputTransform = program.uniform("uOutputTransform");
    entry.outlineBounds = program.uniform("uOutlineBounds");
    entry.outlineRadius = program.uniform("uOutlineRadius");
    entry.outlineWidth = program.uniform("uOutlineWidth");
    entry.continuousCorners = program.uniform("uContinuousCorners");
    m_clipUniforms.push_back(entry);
    return m_clipUniforms.back();
}

bool EglRenderer::makeCurrent(EglRenderTarget *target)
{
    if (!target || !m_context->makeCurrent(target->eglSurface()))
        return false;
    m_lastTarget = target;
    return true;
}

bool EglRenderer::makeContextCurrent()
{
    if (m_current)
        return true;
    if (m_lastTarget)
        return makeCurrent(m_lastTarget);
    // A modeset retires the remembered target before a frame names the next one;
    // uploads go surfaceless meanwhile, as on Chromium's default offscreen surface.
    return m_context->makeCurrent();
}

void EglRenderer::releaseTarget(EglRenderTarget *target)
{
    if (!target || target != m_lastTarget)
        return;
    if (!makeCurrent(target))
        LM_FATAL(lingmoRender) << "Cannot make EGL context current for output texture retirement";
    if (m_backdrop)
        m_backdrop->resetTextures();
    clearTextureCache();
    m_current = nullptr;
    m_lastTarget = nullptr;
    // Left current, the EGLSurface and its GBM colour buffers would live on
    // past gbm_surface_destroy until the next makeCurrent.
    m_context->makeCurrent();
}

void EglRenderer::waitFence(const SyncFence &fence)
{
    m_fences.wait(fence, m_current != nullptr, kFenceWaitMs);
}

SyncFence EglRenderer::createReleaseFence()
{
    return m_fences.release(m_current != nullptr);
}

GpuTimer::Mark EglRenderer::markDrawn()
{
    return m_current ? m_timer.mark() : GpuTimer::Mark();
}

std::optional<GpuTimer::Time> EglRenderer::drawnAt(const GpuTimer::Mark &mark)
{
    if (!mark.isValid() || !makeContextCurrent())
        return std::nullopt;
    return m_timer.reached(mark);
}

std::unique_ptr<Texture> EglRenderer::createTexture(const base::Size &size, const uint8_t *pixels,
                                                    int stride, PixelFormat format)
{
    if (!makeContextCurrent())
        return nullptr;
    return m_upload.create(size, pixels, stride, format);
}

bool EglRenderer::updateTexture(Texture *texture, const uint8_t *pixels, int stride,
                                PixelFormat format)
{
    if (!makeContextCurrent())
        return false;
    return m_upload.update(texture, pixels, stride, format);
}

bool EglRenderer::updateTexture(Texture *texture, const uint8_t *pixels, int stride,
                                PixelFormat format, const base::Region &damage)
{
    if (!makeContextCurrent())
        return false;
    return m_upload.update(texture, pixels, stride, format, damage);
}

std::shared_ptr<Texture> EglRenderer::importDmaBuf(const DmaBufAttributes &attributes)
{
    if (!makeContextCurrent() || !m_externalProgram)
        return nullptr;
    return m_importer.import(attributes);
}

ui::Bitmap EglRenderer::captureFrame(const std::function<void()> &paint)
{
    if (!m_current || m_pass.viewport().isEmpty())
        return {};
    return m_pass.capture(*this, m_current->size(), paint);
}

ui::Bitmap EglRenderer::captureImage(const OutputRenderContext &context, const base::Size &size,
                                     const std::function<void()> &paint)
{
    if (!m_current)
        return {};
    ScopedValue clips(m_roundedClips);
    m_roundedClips.clear();
    return m_pass.captureImage(*this, context, size, paint);
}

std::shared_ptr<PendingImage> EglRenderer::captureFrameLater(const std::function<void()> &paint)
{
    if (!m_current || m_pass.viewport().isEmpty())
        return nullptr;
    return m_pass.readBack(*this, outputContext(), m_current->size(), paint);
}

std::shared_ptr<PendingImage> EglRenderer::captureImageLater(const OutputRenderContext &context,
                                                             const base::Size &size,
                                                             const std::function<void()> &paint)
{
    if (!m_current)
        return nullptr;
    ScopedValue clips(m_roundedClips);
    m_roundedClips.clear();
    return m_pass.readBack(*this, context, size, paint);
}

bool EglRenderer::captureToDmaBuf(const OutputRenderContext &context, const base::Size &size,
                                  const DmaBufAttributes &attributes,
                                  const std::function<void()> &paint)
{
    if (!m_current || attributes.size != size)
        return false;
    const auto &formats = captureDmaBufFormats();
    const auto format = formats.find(attributes.format);
    if (format == formats.end()
        || std::find(format->second.begin(), format->second.end(), attributes.modifier)
                == format->second.end())
        return false;
    EGLImageKHR image = m_importer.createImage(attributes);
    if (image == EGL_NO_IMAGE_KHR)
        return false;
    ScopedValue clips(m_roundedClips);
    m_roundedClips.clear();
    const bool ok = m_pass.captureToImage(*this, context, size, image, paint);
    auto destroyImage =
            reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (destroyImage)
        destroyImage(m_context->display(), image);
    return ok;
}

bool EglRenderer::copyOutputFrame(const base::Size &size)
{
    if (!m_current || !capturing || size != m_current->size()
        || !m_context->glVersion().starts_with("OpenGL ES 3"))
        return false;
    GLint destination = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &destination);
    if (static_cast<unsigned int>(destination) == m_frameFramebuffer)
        return false;
    const bool scissored = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_SCISSOR_TEST);
    // Both are bottom-up, so the rows land as a paint would have put them.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination);
    glBlitFramebuffer(0, 0, size.width(), size.height(), 0, 0, size.width(), size.height(),
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, destination);
    if (scissored)
        glEnable(GL_SCISSOR_TEST);
    return glGetError() == GL_NO_ERROR;
}

// Whether this GPU renders into a buffer allocated the way a capture client
// would allocate it: one probe per format, with a linear layout, so a client
// never learns of a format the frame path would then refuse.
const DmaBufFormats &EglRenderer::captureDmaBufFormats() const
{
    if (m_captureFormats)
        return *m_captureFormats;
    m_captureFormats.emplace();
    if (!m_device)
        return *m_captureFormats;
    ScopedEglCurrent current(*m_context);
    auto imageTargetTexture = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
            eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!current.valid() || !imageTargetTexture)
        return *m_captureFormats;

    const int fd = gbm_device_get_fd(m_device);
    struct stat deviceStat = {};
    if (char *node = drmGetRenderDeviceNameFromFd(fd)) {
        if (stat(node, &deviceStat) == 0)
            m_captureDevice = deviceStat.st_rdev;
        free(node);
    }
    if (!m_captureDevice)
        m_captureDevice = m_importer.mainDevice();

    const auto renderable = [&](gbm_bo *bo, uint32_t format, uint64_t modifier) {
        DmaBufAttributes attributes;
        attributes.size = base::Size(16, 16);
        attributes.format = format;
        attributes.modifier = modifier;
        DmaBufPlane plane;
        plane.fd = ScopedFd(gbm_bo_get_fd(bo));
        plane.stride = gbm_bo_get_stride(bo);
        attributes.planes.push_back(std::move(plane));
        if (attributes.planes.front().fd.get() < 0)
            return false;
        EGLImageKHR image = m_importer.createImage(attributes);
        if (image == EGL_NO_IMAGE_KHR)
            return false;
        GLuint name = 0, framebuffer = 0;
        glGenTextures(1, &name);
        glBindTexture(GL_TEXTURE_2D, name);
        imageTargetTexture(GL_TEXTURE_2D, image);
        glBindTexture(GL_TEXTURE_2D, 0);
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, name, 0);
        const bool ok = glGetError() == GL_NO_ERROR
                && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &name);
        auto destroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(
                eglGetProcAddress("eglDestroyImageKHR"));
        if (destroyImage)
            destroyImage(m_context->display(), image);
        return ok;
    };

    for (const uint32_t format : {DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888}) {
        std::vector<uint64_t> modifiers;
        // Clients that name modifiers (PipeWire consumers such as Sunshine)
        // never offer INVALID, so an explicit LINEAR is needed to meet them.
        const uint64_t linear = DRM_FORMAT_MOD_LINEAR;
        if (gbm_bo *bo = gbm_bo_create_with_modifiers2(m_device, 16, 16, format, &linear, 1,
                                                       GBM_BO_USE_RENDERING)) {
            if (gbm_bo_get_modifier(bo) == DRM_FORMAT_MOD_LINEAR
                && renderable(bo, format, DRM_FORMAT_MOD_LINEAR))
                modifiers.push_back(DRM_FORMAT_MOD_LINEAR);
            gbm_bo_destroy(bo);
        }
        // Some GBM backends (virgl among them) only allocate through the
        // flag, and report the result as INVALID.
        if (gbm_bo *bo = gbm_bo_create(m_device, 16, 16, format,
                                       GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR)) {
            const uint64_t modifier = gbm_bo_get_modifier(bo);
            if (std::find(modifiers.begin(), modifiers.end(), modifier) == modifiers.end()
                && renderable(bo, format, modifier))
                modifiers.push_back(modifier);
            gbm_bo_destroy(bo);
        }
        if (!modifiers.empty())
            m_captureFormats->emplace(format, std::move(modifiers));
    }
    LM_INFO(lingmoRender).printf("Captures can render into %zu DMA-BUF formats",
           m_captureFormats->size());
    return *m_captureFormats;
}

dev_t EglRenderer::captureDmaBufDevice() const
{
    captureDmaBufFormats();
    return m_captureDevice;
}

ui::Bitmap EglRenderer::renderToImage(const base::Size &size, const std::function<void()> &paint)
{
    if (!m_current)
        return {};
    ScopedValue clips(m_roundedClips);
    m_roundedClips.clear();
    return m_pass.renderToImage(*this, size, paint);
}

bool EglRenderer::renderToTexture(std::unique_ptr<Texture> &texture, const base::Rect &bounds,
                                  const std::function<void()> &paint)
{
    return renderToTexture(texture, bounds, base::Rect(), paint);
}

bool EglRenderer::renderToTexture(std::unique_ptr<Texture> &texture, const base::Rect &bounds,
                                  const base::Rect &repaint, const std::function<void()> &paint)
{
    if (!m_current)
        return false;
    return m_pass.render(*this, texture, bounds, paint, repaint);
}

std::unique_ptr<Texture> EglRenderer::renderShadow(const base::Size &size,
                                                   const std::vector<ShadowLobe> &lobes,
                                                   const std::function<void()> &paintCaster)
{
    if (!m_current || !m_dropShadow)
        return nullptr;
    // A clip of the node being painted would cut into the caster.
    ScopedValue clips(m_roundedClips);
    m_roundedClips.clear();
    const double scale = rasterScale();
    setRasterScale(1);
    auto texture = m_dropShadow->render(*this, size, lobes, paintCaster);
    setRasterScale(scale);
    return texture;
}

void EglRenderer::enableTrilinearFiltering(Texture *texture)
{
    if (m_current)
        m_pass.generateMipmaps(texture);
}

void EglRenderer::limitMipmapLevel(Texture *texture, int level)
{
    if (m_current)
        m_pass.limitMipmapLevel(texture, level);
}

void EglRenderer::prepareDamage(const base::Region &damage, const std::vector<base::Rect> &samples)
{
    if (!m_current || capturing || m_current->damagePrepared())
        return;
    applyRepaint(m_current->prepareDamage(damage, samples, outputContext()));
}

bool EglRenderer::repaints(const base::Rect &logical) const
{
    if (!m_current || capturing || !m_repaint)
        return true;
    return m_repaint->intersects(physicalDamage(logical, outputContext(), m_current->size()));
}

bool EglRenderer::endFrame()
{
    if (!m_current)
        return false;
    const bool ok = m_current->swap(m_swapState);
    if (m_backdrop)
        m_backdrop->frameEnded();
    glDisable(GL_SCISSOR_TEST);
    m_repaint.reset();
    paintClip.reset();
    m_current = nullptr;
    return ok;
}

void EglRenderer::invalidateFrame()
{
    if (m_current)
        m_current->invalidate();
}

void EglRenderer::tintFrame(const ColorTransform &transform)
{
    if (!m_current || transform.isIdentity())
        return;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, m_current->size().width(), m_current->size().height());
    glDisable(GL_SCISSOR_TEST);
    static constexpr float vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glUseProgram(m_colorProgram);
    applyRoundedClip(m_colorProgram);
    glUniform4f(m_colorUniform, transform.red, transform.green, transform.blue, 1);
    glVertexAttribPointer(GlProgram::kPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(GlProgram::kPositionAttribute);
    glEnable(GL_BLEND);
    // The destination times the gains, its alpha kept.
    glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisableVertexAttribArray(GlProgram::kPositionAttribute);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    if (m_repaint)
        applyRepaint(*m_repaint);
}

void EglRenderer::cancelFrame()
{
    glDisable(GL_SCISSOR_TEST);
    m_repaint.reset();
    paintClip.reset();
    if (m_current)
        m_current->invalidate();
    m_current = nullptr;
}

void EglRenderer::setOutputContext(OutputRenderContext context)
{
    Renderer::setOutputContext(std::move(context));
    if (m_current)
        beginFrame(m_current);
}

bool EglRenderer::beginFrame(RenderTarget *target)
{
    auto *eglTarget = static_cast<EglRenderTarget *>(target);
    if (!makeCurrent(eglTarget))
        return false;

    m_current = eglTarget;
    const base::Rect content = outputContext().nativeContentRect.isEmpty()
            ? base::Rect(base::Point(), eglTarget->size())
            : outputContext().nativeContentRect;
    m_current->beginFrame();
    GLint framebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
    m_frameFramebuffer = static_cast<unsigned int>(framebuffer);
    m_pass.setGeometry(content.size(), outputContext().logicalViewport.topLeft());
    glViewport(content.x(), eglTarget->size().height() - content.y() - content.height(),
               content.width(), content.height());
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    m_repaint.reset();
    paintClip.reset();
    return true;
}

void EglRenderer::applyRepaint(const base::Rect &repaint)
{
    m_repaint = repaint;
    paintClip = logicalRepaint(repaint, outputContext(), m_current->size());
    glEnable(GL_SCISSOR_TEST);
    glScissor(repaint.x(), m_current->size().height() - repaint.y() - repaint.height(),
              repaint.width(), repaint.height());
}

void EglRenderer::clear(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void EglRenderer::quadFor(const base::RectF &rect, float *vertices) const
{
    quadFor({rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight()}, vertices);
}

void EglRenderer::quadFor(const std::array<base::PointF, 4> &corners, float *vertices) const
{
    const bool quarterTurn = outputContext().transform == 1 || outputContext().transform == 3;
    const float width =
            (quarterTurn ? m_pass.viewport().height() : m_pass.viewport().width()) / rasterScale();
    const float height =
            (quarterTurn ? m_pass.viewport().width() : m_pass.viewport().height()) / rasterScale();
    if (width <= 0.0f || height <= 0.0f)
        return;

    const auto map = [&](const base::PointF &point) {
        base::PointF normalized(point.x() / width, point.y() / height);
        switch (outputContext().transform) {
        case 1:
            normalized = {normalized.y(), 1.0 - normalized.x()};
            break;
        case 2:
            normalized = {1.0 - normalized.x(), 1.0 - normalized.y()};
            break;
        case 3:
            normalized = {1.0 - normalized.y(), normalized.x()};
            break;
        default:
            break;
        }
        return base::PointF(normalized.x() * 2.0 - 1.0, 1.0 - normalized.y() * 2.0);
    };
    for (int index = 0; index < 4; ++index) {
        const base::PointF point = map(corners[index]);
        vertices[index * 2] = point.x();
        vertices[index * 2 + 1] = point.y();
    }
}

void EglRenderer::applyRoundedClip(const GlProgram &program)
{
    ClipUniforms::State state;
    state.count = int(m_roundedClips.size());
    glGetIntegerv(GL_VIEWPORT, state.viewport);
    state.transform = outputContext().transform;
    const auto device = [&](const RoundedShape &shape) {
        return base::RectF((shape.bounds.topLeft() - m_pass.offset()) * rasterScale(),
                      shape.bounds.size() * rasterScale());
    };
    const size_t count = std::min(m_roundedClips.size(), size_t(16));
    for (size_t i = 0; i < count; ++i) {
        const base::RectF rect = device(m_roundedClips[i]);
        state.bounds[i * 4] = rect.x();
        state.bounds[i * 4 + 1] = rect.y();
        state.bounds[i * 4 + 2] = rect.width();
        state.bounds[i * 4 + 3] = rect.height();
        state.radii[i * 2] =
                m_roundedClips[i].radius * m_roundedClips[i].radiusScale.width() * rasterScale();
        state.radii[i * 2 + 1] =
                m_roundedClips[i].radius * m_roundedClips[i].radiusScale.height() * rasterScale();
    }
    const base::RectF outline = device(m_outlineShape);
    state.outline[0] = outline.x();
    state.outline[1] = outline.y();
    state.outline[2] = outline.width();
    state.outline[3] = outline.height();
    state.outlineRadii[0] =
            m_outlineShape.radius * m_outlineShape.radiusScale.width() * rasterScale();
    state.outlineRadii[1] =
            m_outlineShape.radius * m_outlineShape.radiusScale.height() * rasterScale();
    state.outlineWidth = m_outlineWidth;
    state.continuous = continuousCorners() ? 1.0f : 0.0f;

    ClipUniforms &uniforms = clipUniforms(program);
    if (uniforms.uploaded && *uniforms.uploaded == state)
        return;
    const bool clipsChanged = !uniforms.uploaded
            || !std::equal(std::begin(state.bounds), std::end(state.bounds),
                           std::begin(uniforms.uploaded->bounds))
            || !std::equal(std::begin(state.radii), std::end(state.radii),
                           std::begin(uniforms.uploaded->radii));
    glUniform1i(uniforms.count, state.count);
    glUniform4f(uniforms.viewportRect, state.viewport[0], state.viewport[1], state.viewport[2],
                state.viewport[3]);
    glUniform1i(uniforms.outputTransform, int(state.transform));
    if (clipsChanged) {
        glUniform4fv(uniforms.clips, 16, state.bounds);
        glUniform2fv(uniforms.clipRadii, 16, state.radii);
    }
    glUniform4f(uniforms.outlineBounds, state.outline[0], state.outline[1], state.outline[2],
                state.outline[3]);
    glUniform2f(uniforms.outlineRadius, state.outlineRadii[0], state.outlineRadii[1]);
    glUniform1f(uniforms.outlineWidth, state.outlineWidth);
    glUniform1f(uniforms.continuousCorners, state.continuous);
    uniforms.uploaded = state;
}

void EglRenderer::drawRoundedOutline(const RoundedShape &shape, const Color &color, double widthPx)
{
    if (widthPx <= 0)
        return;
    m_outlineShape = shape;
    m_outlineWidth = widthPx;
    const double fringe = 0.5 / rasterScale();
    const base::RectF bounds = shape.bounds.adjusted(-fringe, -fringe, fringe, fringe);
    const double bandX =
            std::min(bounds.width() / 2,
                     std::max(shape.radius * shape.radiusScale.width(), widthPx / rasterScale())
                             + 2 * fringe);
    const double bandY =
            std::min(bounds.height() / 2,
                     std::max(shape.radius * shape.radiusScale.height(), widthPx / rasterScale())
                             + 2 * fringe);
    // One draw with disjoint strips avoids shading the transparent window interior.
    fillColorRects(
            {{bounds.x(), bounds.y(), bounds.width(), bandY},
             {bounds.x(), bounds.bottom() - bandY, bounds.width(), bandY},
             {bounds.x(), bounds.y() + bandY, bandX, bounds.height() - 2 * bandY},
             {bounds.right() - bandX, bounds.y() + bandY, bandX, bounds.height() - 2 * bandY}},
            color);
    m_outlineWidth = 0;
}

void EglRenderer::fillRect(const base::Rect &rect, const Color &color)
{
    fillRectF(rect, color);
}

void EglRenderer::fillRectF(const base::RectF &rect, const Color &color)
{
    fillColorRects({rect}, color);
}

void EglRenderer::fillColorRects(std::initializer_list<base::RectF> rects, const Color &color)
{
    if (!m_current)
        return;
    assert(rects.size() <= 4);
    float vertices[48] = {};
    int count = 0;
    for (const auto &rect : rects) {
        if (rect.isEmpty())
            continue;
        float quad[8];
        quadFor(rect.translated(-m_pass.offset()), quad);
        for (int index : {0, 1, 2, 2, 1, 3}) {
            vertices[count++] = quad[index * 2];
            vertices[count++] = quad[index * 2 + 1];
        }
    }
    if (!count)
        return;

    glUseProgram(m_colorProgram);
    applyRoundedClip(m_colorProgram);
    glUniform4f(m_colorUniform, color.r * color.a, color.g * color.a, color.b * color.a, color.a);
    glVertexAttribPointer(GlProgram::kPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(GlProgram::kPositionAttribute);

    if (color.a < 1.0f || !m_roundedClips.empty() || m_outlineWidth > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    glDrawArrays(GL_TRIANGLES, 0, count / 2);

    glDisable(GL_BLEND);
    glDisableVertexAttribArray(GlProgram::kPositionAttribute);
}

void EglRenderer::drawTexture(Texture *texture, const base::Rect &target, float alpha)
{
    drawTextureF(texture, target, alpha);
}

void EglRenderer::drawTextureF(Texture *texture, const base::RectF &target, float alpha)
{
    drawTextureTransformed(texture, target, base::AffineTransform(), alpha);
}

void EglRenderer::drawTextureTransformed(Texture *texture, const base::RectF &target,
                                         const base::AffineTransform &transform, float alpha)
{
    base::AffineTransform mapping;
    if (auto *gl = dynamic_cast<GlTexture *>(texture); gl && !gl->contentPixels.isEmpty()) {
        mapping.translate(gl->contentPixels.x() / gl->size().width(),
                          gl->contentPixels.y() / gl->size().height());
        mapping.scale(gl->contentPixels.width() / gl->size().width(),
                      gl->contentPixels.height() / gl->size().height());
    }
    drawTextureMappedF(texture, target, alpha, mapping, transform);
}

void EglRenderer::drawTextureMapped(Texture *texture, const base::Rect &target, float alpha,
                                    const base::AffineTransform &mapping)
{
    base::RectF aligned;
    base::AffineTransform texelMapping;
    alignTextureDraw(texture, target, mapping, aligned, texelMapping);
    drawTextureMappedF(texture, aligned, alpha, texelMapping);
}

void EglRenderer::drawTextureMappedRects(Texture *texture, const std::vector<MappedRect> &rects,
                                         float alpha)
{
    auto *glTexture = dynamic_cast<GlTexture *>(texture);
    if (!m_current || !glTexture)
        return;
    constexpr size_t kBatch = 16;
    TextureQuad quads[kBatch];
    size_t count = 0;
    for (const MappedRect &rect : rects) {
        base::RectF aligned;
        base::AffineTransform texelMapping;
        alignTextureDraw(texture, rect.target, rect.mapping, aligned, texelMapping);
        if (aligned.isEmpty())
            continue;
        textureQuad(*glTexture, aligned, texelMapping, base::AffineTransform(), quads[count++]);
        if (count == kBatch) {
            drawTextureQuads(glTexture, quads, count, alpha);
            count = 0;
        }
    }
    if (count)
        drawTextureQuads(glTexture, quads, count, alpha);
}

void EglRenderer::alignTextureDraw(const Texture *texture, const base::RectF &target,
                                   const base::AffineTransform &mapping, base::RectF &aligned,
                                   base::AffineTransform &texelMapping) const
{
    aligned = target;
    texelMapping = mapping;
    if (texture && rasterScale() > 0) {
        const base::Size pixels = texture->size();
        const auto sourcePoint = [&](const base::PointF &point) {
            const base::PointF uv = mapping.map(point);
            return base::PointF(uv.x() * pixels.width(), uv.y() * pixels.height());
        };
        const base::PointF source = sourcePoint({0, 0});
        const base::PointF horizontal = sourcePoint({1, 0}) - source;
        const base::PointF vertical = sourcePoint({0, 1}) - source;
        const base::SizeF sourceSize(std::hypot(horizontal.x(), horizontal.y()),
                                std::hypot(vertical.x(), vertical.y()));
        const base::Size deviceSize(base::roundToInt(target.width() * rasterScale()),
                               base::roundToInt(target.height() * rasterScale()));
        const auto equalPixel = [](double a, double b) {
            return std::abs(a - b) < 0.001;
        };
        // A size the client chose in device pixels, like an Xwayland popup,
        // need not be a whole number of logical units: the rounded logical size
        // is then a pixel off. Texels still land one per pixel; the last row or
        // column is repeated from the texture edge, or left out.
        const auto nearPixel = [&](double a, double b) {
            return equalPixel(a, std::round(a)) && std::abs(a - b) < 1.001;
        };
        if (nearPixel(sourceSize.width(), deviceSize.width())
            && nearPixel(sourceSize.height(), deviceSize.height())
            && equalPixel(source.x(), std::round(source.x()))
            && equalPixel(source.y(), std::round(source.y()))) {
            if (!equalPixel(sourceSize.width(), deviceSize.width())
                || !equalPixel(sourceSize.height(), deviceSize.height()))
                texelMapping = base::AffineTransform::fromScale(deviceSize.width() / sourceSize.width(),
                                                     deviceSize.height() / sourceSize.height())
                        * mapping;
            // Like Exo's host origin adjustment, keep native-density texels on physical pixels.
            aligned = base::RectF(pixelAligned(target.topLeft()), base::SizeF(deviceSize) / rasterScale());
        }
    }
}

void EglRenderer::textureQuad(const GlTexture &texture, const base::RectF &target,
                              const base::AffineTransform &mapping, const base::AffineTransform &geometry,
                              TextureQuad &quad) const
{
    const auto corner = [&](const base::PointF &point) {
        return geometry.map(point) - m_pass.offset();
    };
    quadFor({corner(target.topLeft()), corner(target.topRight()), corner(target.bottomLeft()),
             corner(target.bottomRight())},
            quad.vertices);
    // Matches the vertex order in quadFor(); the image origin is top left.
    for (int i = 0; i < 4; ++i) {
        const base::PointF uv = mapping.map(base::PointF(i % 2, i / 2));
        quad.texCoords[2 * i] = uv.x();
        quad.texCoords[2 * i + 1] = texture.inverted ? 1 - uv.y() : uv.y();
    }
}

void EglRenderer::drawTextureMappedF(Texture *texture, const base::RectF &target, float alpha,
                                     const base::AffineTransform &mapping, const base::AffineTransform &geometry)
{
    auto *glTexture = dynamic_cast<GlTexture *>(texture);
    if (!m_current || !glTexture || target.isEmpty())
        return;
    TextureQuad quad;
    textureQuad(*glTexture, target, mapping, geometry, quad);
    drawTextureQuads(glTexture, &quad, 1, alpha);
}

void EglRenderer::drawTextureQuads(GlTexture *glTexture, const TextureQuad *quads, size_t count,
                                   float alpha)
{
    if (!count)
        return;
    const bool external = glTexture->target() == GL_TEXTURE_EXTERNAL_OES;
    if (external && !m_externalProgram)
        return;

    // Each quad as two triangles, the same two a strip of its corners makes.
    float vertices[16 * 12];
    float texCoords[16 * 12];
    int filled = 0;
    for (size_t q = 0; q < count; ++q) {
        for (int index : {0, 1, 2, 2, 1, 3}) {
            vertices[filled] = quads[q].vertices[index * 2];
            texCoords[filled++] = quads[q].texCoords[index * 2];
            vertices[filled] = quads[q].vertices[index * 2 + 1];
            texCoords[filled++] = quads[q].texCoords[index * 2 + 1];
        }
    }

    const GlProgram &program = external ? m_externalProgram : m_textureProgram;
    TextureUniforms &uniforms = external ? m_externalUniforms : m_textureUniforms;
    glUseProgram(program);
    applyRoundedClip(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(glTexture->target(), glTexture->name());
    const bool hasAlpha = pixelFormatHasAlpha(glTexture->format()) && !glTexture->contentOpaque;
    // An XRGB buffer drawn whole shades to alpha 1; blending it only costs a read.
    const bool opaque = !hasAlpha && (alpha >= 1 || overwriteOpaque)
            && m_roundedClips.empty() && m_outlineWidth <= 0;
    // Darkened in place, the colour is scaled in the blend stage and alpha stays
    // 1: a blur reading a pixel left below 1 lets the sharp backdrop through.
    const bool darkened =
            opaque && alpha < 1 && m_pass.blendMode() != GlRenderPass::BlendMode::Copy;
    const float shaderAlpha = darkened ? 1.0f : alpha;
    if (shaderAlpha != uniforms.lastAlpha) {
        glUniform1f(uniforms.alpha, shaderAlpha);
        uniforms.lastAlpha = shaderAlpha;
    }
    const float swizzle = !external && pixelFormatIsBgr(glTexture->format()) ? 1.0f : 0.0f;
    if (!external && swizzle != uniforms.lastSwizzle) {
        glUniform1f(uniforms.swizzle, swizzle);
        uniforms.lastSwizzle = swizzle;
    }
    if (float(hasAlpha) != uniforms.lastHasAlpha) {
        glUniform1f(uniforms.hasAlpha, hasAlpha ? 1.0f : 0.0f);
        uniforms.lastHasAlpha = float(hasAlpha);
    }

    const GLuint positionAttr = GlProgram::kPositionAttribute;
    const GLuint coordAttr = GlProgram::kTexCoordAttribute;
    glVertexAttribPointer(positionAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionAttr);
    glVertexAttribPointer(coordAttr, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    glEnableVertexAttribArray(coordAttr);

    if (darkened) {
        glEnable(GL_BLEND);
        glBlendColor(alpha, alpha, alpha, 1);
        glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_ZERO, GL_ONE, GL_ZERO);
    } else if (m_pass.blendMode() == GlRenderPass::BlendMode::Copy || opaque) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    glDrawArrays(GL_TRIANGLES, 0, filled / 2);

    glDisable(GL_BLEND);
    glDisableVertexAttribArray(positionAttr);
    glDisableVertexAttribArray(coordAttr);
    glBindTexture(glTexture->target(), 0);
}

bool EglRenderer::keepsBackdrop(uint64_t scope, uint64_t owner) const
{
    return m_backdrop && m_backdrop->keeps(scope, owner);
}

void EglRenderer::beginBackdropPass(uint64_t scope, const std::vector<uint64_t> &fresh)
{
    if (m_backdrop)
        m_backdrop->beginPass(scope, fresh);
}

void EglRenderer::drawBackgroundBlur(const base::Rect &bounds, int radius, const base::Rect &clip,
                                     const GlassParams &glass, const ui::Path &shape,
                                     uint64_t owner)
{
    if (!m_current || !m_backdrop)
        return;
    const double scale = rasterScale();
    const auto coverage = [scale](const base::Rect &rect) {
        return base::RectF(base::PointF(rect.topLeft()) * scale, base::SizeF(rect.size()) * scale).toEnclosingRect();
    };
    ScopedValue clips(m_roundedClips);
    for (auto &shape : m_roundedClips) {
        shape.bounds = base::RectF(shape.bounds.topLeft() * scale, shape.bounds.size() * scale);
        shape.radius *= scale;
    }
    const auto sources = m_pass.backdropSources(scale, outputContext().transform);
    const base::PointF offset = m_pass.offset();
    // `clip` is the surface; its texture lands on physical pixels, so the body does too.
    const base::PointF snap = pixelAligned(clip.topLeft()) - base::PointF(clip.topLeft());

    struct RestoreGeometry {
        EglRenderer &renderer;
        GlRenderPass &pass;
        base::PointF offset;
        double scale;

        ~RestoreGeometry()
        {
            pass.setGeometry(pass.viewport(), offset);
            renderer.setRasterScale(scale);
        }
    } restore {*this, m_pass, offset, scale};

    m_pass.setGeometry(m_pass.viewport(), offset * scale);
    setRasterScale(1);
    m_backdrop->draw(
            *this, base::RectF((bounds.topLeft() + snap) * scale, base::SizeF(bounds.size()) * scale),
            base::roundToInt(radius * scale), coverage(clip), glass, scale, shape.translated(snap.x(), snap.y()),
            sources, m_roundedClips,
            [this](const base::RectF &rect, float *vertices) {
                quadFor(rect.translated(-m_pass.offset()), vertices);
            },
            [this](const GlProgram &program) {
                applyRoundedClip(program);
            },
            backdropScope, owner);
}

} // namespace Lingmo::Render

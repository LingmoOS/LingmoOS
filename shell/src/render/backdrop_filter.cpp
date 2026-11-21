// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/backdrop_filter.h"

#include <algorithm>
#include <cmath>

#include "render/damage.h"
#include "render/gaussian_kernel.h"
#include "render/shape_mask.h"
#include "render_shaders.h"
#include <cassert>
#include "base/geometry.h"
#include "base/transform.h"
#include "ui/geometry_conversions.h"

namespace Lingmo::Render {

bool BackdropFilter::initialize(const char *fragmentOverride)
{
    GlProgram blur =
            GlProgram::create(shaders::texture_vertex_shader,
                              fragmentOverride ? fragmentOverride : shaders::blur_fragment_shader);
    GlProgram mask = GlProgram::create(
            shaders::texture_vertex_shader,
            (fragmentOverride ? std::string(fragmentOverride)
                              : clippedFragment(shaders::blur_mask_fragment_shader))
                    .c_str());
    GlProgram glass =
            GlProgram::create(shaders::texture_vertex_shader,
                              (fragmentOverride ? std::string(fragmentOverride)
                                                : clippedFragment(shaders::glass_fragment_shader))
                                      .c_str());
    if (!blur || !mask || !glass)
        return false;
    m_blurProgram = std::move(blur);
    m_blurMaskProgram = std::move(mask);
    m_glassProgram = std::move(glass);
    return true;
}

void BackdropFilter::resetTextures()
{
    m_scratch.clear();
    m_masks.clear();
    m_kept.clear();
}

void BackdropFilter::frameEnded()
{
    ++m_frame;
    const auto stale = [this](const auto &entry) {
        return m_frame - entry.used > kUnusedFrames;
    };
    m_scratch.remove_if(stale);
    m_masks.remove_if(stale);
    m_kept.remove_if(stale);
}

bool BackdropFilter::keeps(uint64_t scope, uint64_t owner) const
{
    return std::any_of(m_kept.begin(), m_kept.end(), [&](const Kept &entry) {
        return entry.scope == scope && entry.owner == owner && entry.result;
    });
}

void BackdropFilter::beginPass(uint64_t scope, const std::vector<uint64_t> &fresh)
{
    m_kept.remove_if([&](const Kept &entry) {
        return entry.scope == scope
                && std::find(fresh.begin(), fresh.end(), entry.owner) == fresh.end();
    });
    for (Kept &entry : m_kept) {
        if (entry.scope != scope)
            continue;
        entry.drawn = false;
        entry.used = m_frame;
    }
}

BackdropFilter::Scratch &BackdropFilter::scratch(const base::Size &coverage, const base::Size &size)
{
    auto found = std::find_if(m_scratch.begin(), m_scratch.end(), [&](const Scratch &entry) {
        return entry.coverage == coverage && entry.size == size;
    });
    // A plain blur takes the smallest set it fits in, so the windows of the
    // overview share a few sets rather than each keeping its own.
    if (coverage.isEmpty()) {
        for (auto entry = m_scratch.begin(); entry != m_scratch.end(); ++entry) {
            if (!entry->coverage.isEmpty() || entry->size.width() < size.width()
                || entry->size.height() < size.height())
                continue;
            if (found == m_scratch.end()
                || entry->size.width() * entry->size.height()
                        < found->size.width() * found->size.height())
                found = entry;
        }
    }
    if (found == m_scratch.end()) {
        if (m_scratch.size() == kCachedSizes)
            m_scratch.pop_back();
        m_scratch.push_front({coverage, size});
    } else {
        m_scratch.splice(m_scratch.begin(), m_scratch, found);
    }
    m_scratch.front().used = m_frame;
    return m_scratch.front();
}

Texture *BackdropFilter::mask(Renderer &renderer, const ui::Path &shape,
                              const ShapeRaster &raster)
{
    auto found = std::find_if(m_masks.begin(), m_masks.end(), [&](const Mask &entry) {
        return entry.raster == raster && entry.shape == shape;
    });
    if (found == m_masks.end()) {
        const ui::Bitmap image = rasterizeShape(shape, raster);
        auto texture = renderer.createTexture(image.size(), image.constBits(), int(image.stride()),
                                              PixelFormat::Rgba8888);
        if (!texture)
            return nullptr;
        if (m_masks.size() == kCachedSizes)
            m_masks.pop_back();
        m_masks.push_front({shape, raster, 0, std::move(texture)});
    } else {
        m_masks.splice(m_masks.begin(), m_masks, found);
    }
    m_masks.front().used = m_frame;
    return m_masks.front().texture.get();
}

void BackdropFilter::applyGlassMaterial(const GlProgram &program, const GlassParams &glass,
                                        const base::Size &coverage, double scale)
{
    const auto uniform = [&program](const char *name) {
        return program.uniform(name);
    };
    glUniform2f(uniform("uCoverage"), coverage.width(), coverage.height());
    glUniform1f(uniform("uBevel"), std::max(glass.bevel, 0.0f) * scale);
    glUniform1f(uniform("uDepth"), std::max(glass.depth, 0.0f) * scale);
    glUniform1f(uniform("uEta"), 1.0f / std::max(glass.ior, 1.001f));
    glUniform1f(uniform("uProfile"), std::max(glass.profile, 1.01f));
    glUniform1f(uniform("uDisplacement"), glass.displacement * scale);
    glUniform1f(uniform("uChroma"), std::clamp(glass.chroma, 0.0f, 1.0f));
    // Surface-local y grows downwards; the z term places the light above the
    // glass rather than edge-on to it.
    const float angle = float(glass.lightAngle * M_PI / 180.0);
    const float light[3] = {std::cos(angle), std::sin(angle), 0.38f};
    const float length = std::sqrt(light[0] * light[0] + light[1] * light[1] + light[2] * light[2]);
    glUniform3f(uniform("uLight"), light[0] / length, light[1] / length, light[2] / length);
    glUniform3f(uniform("uSpecular"), std::max(glass.specularIntensity, 0.0f),
                std::max(glass.shininess, 1.0f), std::max(glass.sheen, 0.0f));
    glUniform2f(uniform("uAmbient"), std::clamp(glass.ambientOcclusion, 0.0f, 1.0f),
                std::max(glass.ambientOcclusionRadius, 0.001f) * scale);
    glUniform1f(uniform("uSaturation"), std::max(glass.saturation, 0.0f));
    // The shader derives the hairline widths from one logical pixel.
    glUniform2f(uniform("uRim"), std::clamp(glass.rim, 0.0f, 1.0f), std::max(1.0f, float(scale)));
    glUniform4f(uniform("uTint"), glass.tintRed, glass.tintGreen, glass.tintBlue,
                std::clamp(glass.tintAlpha, 0.0f, 1.0f));
}

bool BackdropFilter::blitReduced(const std::vector<BackdropSource> &sources, const base::Rect &coverage,
                                 const base::Size &reduced, unsigned int destination)
{
    if (sources.size() != 1 || sources.front().transform
        || !m_context->glVersion().starts_with("OpenGL ES 3"))
        return false;
    const BackdropSource &source = sources.front();
    if (!base::Rect(source.offset.toPoint(), source.oriented()).contains(coverage))
        return false;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] < reduced.width() || viewport[3] < reduced.height())
        return false;
    const base::Rect physical = source.framebufferRect(coverage);
    const bool scissored = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_SCISSOR_TEST);
    // Both are bottom-up, as the capture and the draw of it would leave them;
    // the pass's top left is the top of its rows.
    const int top = viewport[1] + viewport[3];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination);
    glBlitFramebuffer(physical.x(), physical.y(), physical.x() + physical.width(),
                      physical.y() + physical.height(), viewport[0], top - reduced.height(),
                      viewport[0] + reduced.width(), top, GL_COLOR_BUFFER_BIT,
                      reduced == coverage.size() ? GL_NEAREST : GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, destination);
    if (scissored)
        glEnable(GL_SCISSOR_TEST);
    return glGetError() == GL_NO_ERROR;
}

bool BackdropFilter::captureBackdrop(Renderer &renderer, Scratch &textures,
                                     const std::vector<BackdropSource> &sources,
                                     const base::Rect &coverage, const base::Size &reduced,
                                     const base::Rect &allocatedBounds)
{
    const base::Rect reducedBounds(base::Point(), reduced);
    bool copied = true;
    return renderer.renderToTexture(
                textures.source, allocatedBounds,
                [&] {
                    GLint destination = 0;
                    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &destination);
                    if (blitReduced(sources, coverage, reduced, destination))
                        return;
                    for (const auto &source : sources) {
                        const base::Rect area = coverage.intersected(
                                base::Rect(source.offset.toPoint(), source.oriented()));
                        if (area.isEmpty())
                            continue;
                        const base::Rect physical = source.framebufferRect(area);
                        // A turned framebuffer is copied as it lies and drawn upright below.
                        const base::Size captureSize =
                                source.transform ? physical.size() : coverage.size();
                        glBindFramebuffer(GL_FRAMEBUFFER, source.framebuffer);
                        GLint alphaBits = 0;
                        glGetIntegerv(GL_ALPHA_BITS, &alphaBits);
                        glBindFramebuffer(GL_FRAMEBUFFER, destination);
                        GlTexture *capture = nullptr;
                        if (alphaBits == 0) {
                            if (!textures.opaqueCapture
                                || textures.opaqueCapture->size() != captureSize) {
                                GLuint name = 0;
                                glGenTextures(1, &name);
                                glBindTexture(GL_TEXTURE_2D, name);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, captureSize.width(),
                                             captureSize.height(), 0, GL_RGB, GL_UNSIGNED_BYTE,
                                             nullptr);
                                textures.opaqueCapture = std::make_unique<GlTexture>(
                                        name, captureSize, PixelFormat::Rgbx8888, GL_TEXTURE_2D,
                                        m_context);
                                static_cast<GlTexture *>(textures.opaqueCapture.get())->inverted =
                                        true;
                            }
                            capture = static_cast<GlTexture *>(textures.opaqueCapture.get());
                        } else {
                            if (!renderer.renderToTexture(
                                        textures.capture, base::Rect(base::Point(), captureSize), [] { })) {
                                copied = false;
                                break;
                            }
                            capture = static_cast<GlTexture *>(textures.capture.get());
                        }
                        // Match RGB outputs when copying; virgl cannot copy them into RGBA.
                        glBindFramebuffer(GL_FRAMEBUFFER, source.framebuffer);
                        glBindTexture(GL_TEXTURE_2D, capture->name());
                        if (source.transform) {
                            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, physical.x(), physical.y(),
                                                physical.width(), physical.height());
                        } else {
                            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, area.x() - coverage.x(),
                                                coverage.height() - (area.y() - coverage.y())
                                                        - area.height(),
                                                physical.x(), physical.y(), physical.width(),
                                                physical.height());
                        }
                        glBindFramebuffer(GL_FRAMEBUFFER, destination);
                        if (!source.transform) {
                            renderer.drawTexture(capture, reducedBounds, 1);
                            continue;
                        }
                        const double sx = double(reduced.width()) / coverage.width();
                        const double sy = double(reduced.height()) / coverage.height();
                        const base::RectF upright((area.x() - coverage.x()) * sx,
                                             (area.y() - coverage.y()) * sy, area.width() * sx,
                                             area.height() * sy);
                        base::RectF lying(base::PointF(), source.transform % 2 ? upright.size().transposed()
                                                                     : upright.size());
                        lying.moveCenter(upright.center());
                        // Undoes the output's turn, as ScreenTransitionNode does.
                        base::AffineTransform turn;
                        turn.translate(upright.center().x(), upright.center().y());
                        turn.rotate(source.transform == 1 ? 90 : source.transform == 2 ? 180 : -90);
                        turn.translate(-upright.center().x(), -upright.center().y());
                        renderer.drawTextureTransformed(capture, lying, turn, 1);
                    }
                })
            && copied;
}

void BackdropFilter::draw(Renderer &renderer, const base::RectF &exactLens, int radius,
                          const base::Rect &clip, const GlassParams &glass, double scale,
                          const ui::Path &shape, const std::vector<BackdropSource> &sources,
                          std::vector<RoundedShape> &clips,
                          const std::function<void(const base::RectF &, float *)> &quadFor,
                          const std::function<void(const GlProgram &)> &applyRoundedClip,
                          uint64_t scope, uint64_t owner)
{
    assert(!sources.empty());
    const BackdropSource &target = sources.back();
    const base::Rect lens = exactLens.toEnclosingRect();
    const bool shaped = !shape.isEmpty();
    // A shaped glass draws over its whole outline, with the lens inside it.
    const base::Rect bounds = glass.enabled && shaped
            ? lens.united(ui::toBase(shape.boundingRect()).scaled(scale).toEnclosingRect())
            : lens;
    if (bounds.isEmpty() || !bounds.intersects(clip))
        return;

    // Sample beyond the mask; the sigma floor keeps the one-tap case finite.
    const float sigma =
            (glass.enabled ? std::max(glass.blurSigma, 0.05f) : std::max(glass.plainSigma, 0.05f))
            * scale;
    // Glass samples the backdrop per output pixel, so it wants device
    // resolution until its sigma passes Skia's kMaxLinearSigma, past which
    // Skia halves the resolution too; the plain blur is specified in logical pixels.
    float quality = glass.enabled ? 1.0f : 0.33f / scale;
    while (glass.enabled && sigma * quality > kMaxGlassSigma)
        quality /= 2;
    const base::Rect output(sources.front().offset.toPoint(), sources.front().oriented());
    const base::Rect visibleBounds = bounds.intersected(clip).intersected(output);
    if (visibleBounds.isEmpty())
        return;
    // Rounded outward, the lens is a pixel wider or taller depending on where
    // its fractional origin falls, and so is a shape's outline. The capture is
    // sized for the widest they can round out to, so a lens moving at a
    // fractional scale keeps one set of scratch textures instead of cycling
    // through four and reallocating, however the clip or the output cut it.
    // cc's ResourcePool hands out its unused resources by exact size the same way.
    base::Size captured = visibleBounds.size();
    if (scale != std::floor(scale)) {
        base::RectF exact = exactLens;
        if (bounds != lens)
            exact = exact.united(ui::toBase(shape.boundingRect()).scaled(scale));
        captured = captured.expandedTo(
                base::Size(int(std::ceil(exact.width())) + 1, int(std::ceil(exact.height())) + 1));
    }
    // Three sigmas, kBackdropBlurSupport at the plain blur's full size.
    const float support = glass.enabled
            ? float(kBackdropBlurSupport)
            : std::min(3 * glass.plainSigma, float(kBackdropBlurSupport));
    const int halo = base::ceilToInt(support * scale);
    // Moved inside the output rather than cut by its edge, so the size stays
    // the same as the lens nears the edge: cut, it keyed a new set of scratch
    // textures on every frame of a move along it. Only a lens the output
    // cannot hold with its halo is cut. Skia sizes such scratch textures
    // loosely for the same reason (SkBackingFit::kApprox).
    base::Rect coverage(visibleBounds.topLeft() - base::Point(halo, halo),
                   captured + base::Size(2 * halo, 2 * halo));
    coverage.moveLeft(std::clamp(coverage.left(), output.left(),
                                 std::max(output.left(), output.right() - coverage.width())));
    coverage.moveTop(std::clamp(coverage.top(), output.top(),
                                std::max(output.top(), output.bottom() - coverage.height())));
    coverage = coverage.intersected(output);
    const base::Size reduced(std::max(1, int(std::ceil(coverage.width() * quality))),
                        std::max(1, int(std::ceil(coverage.height() * quality))));
    const base::Rect reducedBounds(base::Point(), reduced);

    GLint oldScissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, oldScissor);
    const bool wasScissored = glIsEnabled(GL_SCISSOR_TEST);
    base::Rect scissor = target.framebufferRect(
            clip.intersected(base::Rect(target.offset.toPoint(), target.oriented())));
    if (wasScissored)
        scissor = scissor.intersected(
                base::Rect(oldScissor[0], oldScissor[1], oldScissor[2], oldScissor[3]));
    // Outside a partial repaint the blur would only be scissored away.
    if (scissor.intersected(target.framebufferRect(bounds)).isEmpty())
        return;

    Kept *kept = nullptr;
    if (scope && owner) {
        const auto found = std::find_if(m_kept.begin(), m_kept.end(), [&](const Kept &entry) {
            return entry.scope == scope && entry.owner == owner;
        });
        if (found == m_kept.end()) {
            m_kept.push_front({scope, owner});
            kept = &m_kept.front();
        } else if (!found->drawn) {
            kept = &*found;
        }
        if (kept) {
            kept->drawn = true;
            kept->used = m_frame;
        }
    }
    const bool reused = kept && kept->result && kept->coverage == coverage
            && kept->reduced == reduced && kept->sigma == sigma;
    // A plain blur shrinks with its window on every frame of the overview.
    // Sized up to a step, or in a larger set, its textures last the whole
    // move; the passes use their top left and clamp their samples to it.
    // Glass keeps exact sizes.
    const auto stepUp = [](int value) {
        return (value + kScratchStep - 1) / kScratchStep * kScratchStep;
    };
    Scratch *textures = nullptr;
    if (!reused)
        textures = glass.enabled
                ? &scratch(coverage.size(), reduced)
                : &scratch(base::Size(), base::Size(stepUp(reduced.width()), stepUp(reduced.height())));
    const base::Size allocated = reused ? kept->allocated : textures->size;
    const base::Rect allocatedBounds(base::Point(), allocated);
    // The used texels, rows bottom-up, and their share of the texture.
    const base::Rect texels(0, allocated.height() - reduced.height(), reduced.width(),
                       reduced.height());
    const base::SizeF used(double(reduced.width()) / allocated.width(),
                      double(reduced.height()) / allocated.height());

    struct RestoreClips {
        std::vector<RoundedShape> &slot;
        std::vector<RoundedShape> saved;

        ~RestoreClips() { slot = std::move(saved); }
    } restoreClips {clips, std::exchange(clips, {})};

    std::unique_ptr<Texture> &result = kept ? kept->result : textures->result;
    if (!reused
        && !captureBackdrop(renderer, *textures, sources, coverage, reduced, allocatedBounds))
        return;

    const auto drawQuad = [&](const GlProgram &program, Texture *input, const base::Rect &target,
                              bool inverted, const base::SizeF &share = {1, 1}) {
        const auto *texture = static_cast<GlTexture *>(input);
        float vertices[8];
        quadFor(target, vertices);
        const float u = float(share.width());
        const float v = float(share.height());
        const float coordinates[8] = {0, 0, u, 0, 0, v, u, v};
        const float flipped[8] = {0, 1, u, 1, 0, 1 - v, u, 1 - v};
        const GLuint position = GlProgram::kPositionAttribute;
        const GLuint coord = GlProgram::kTexCoordAttribute;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->name());
        glUniform1i(program.uniform("uTexture"), 0);
        glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, 0, inverted ? flipped : coordinates);
        glEnableVertexAttribArray(position);
        glEnableVertexAttribArray(coord);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(position);
        glDisableVertexAttribArray(coord);
        glBindTexture(GL_TEXTURE_2D, 0);
    };
    const auto filter = [&](Texture *input, std::unique_ptr<Texture> &result, bool horizontal) {
        return renderer.renderToTexture(result, allocatedBounds, [&] {
            glUseProgram(m_blurProgram);
            glDisable(GL_BLEND);
            const float scaledSigma = sigma
                    * (horizontal ? float(reduced.width()) / coverage.width()
                                  : float(reduced.height()) / coverage.height());
            setGaussianKernel(m_blurProgram, scaledSigma,
                              horizontal ? 1.0f / allocated.width() : 0,
                              horizontal ? 0 : 1.0f / allocated.height());
            setGaussianClamp(m_blurProgram, texels, allocated);
            drawQuad(m_blurProgram, input, reducedBounds, true, used);
        });
    };
    if (!reused) {
        if (kept)
            kept->coverage = base::Rect();
        if (!filter(textures->source.get(), textures->temp, true)
            || !filter(textures->temp.get(), result, false))
            return;
        if (kept) {
            kept->coverage = coverage;
            kept->reduced = reduced;
            kept->allocated = allocated;
            kept->sigma = sigma;
        }
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissor.x(), scissor.y(), std::max(0, scissor.width()),
              std::max(0, scissor.height()));
    clips = restoreClips.saved;
    if (glass.enabled && !shaped) {
        clips.push_back(
                {base::RectF(bounds),
                 double(std::clamp(radius, 0, std::min(bounds.width(), bounds.height()) / 2))});
    }
    const GlProgram &program = glass.enabled ? m_glassProgram : m_blurMaskProgram;
    glUseProgram(program);
    applyRoundedClip(program);
    const float u = float(used.width());
    const float v = float(used.height());
    glUniform4f(program.uniform("uSampleRect"),
                float(bounds.x() - coverage.x()) / coverage.width() * u,
                float(bounds.y() - coverage.y()) / coverage.height() * v,
                float(bounds.width()) / coverage.width() * u,
                float(bounds.height()) / coverage.height() * v);
    setGaussianClamp(program, texels, allocated);
    glUniform2f(program.uniform("uSize"), bounds.width(), bounds.height());
    glUniform1f(program.uniform("uRadius"),
                std::clamp(radius, 0, std::min(lens.width(), lens.height()) / 2));
    if (!glass.enabled)
        glUniform1f(program.uniform("uOpacity"), std::clamp(glass.opacity, 0.0f, 1.0f));
    if (glass.enabled) {
        applyGlassMaterial(program, glass, coverage.size(), scale);
        glUniform2f(program.uniform("uBackdropSize"), reduced.width(), reduced.height());
        glUniform4f(program.uniform("uLens"), lens.x() - bounds.x(), lens.y() - bounds.y(),
                    lens.width(), lens.height());
    }
    glUniform1f(program.uniform("uHasShape"), shaped ? 1 : 0);
    if (shaped) {
        const base::PointF origin = ui::toBase(shape.boundingRect()).topLeft();
        Texture *shapeMask =
                mask(renderer, shape.translated(-origin.x(), -origin.y()),
                     {scale, origin * scale - base::PointF(bounds.topLeft()), bounds.size()});
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shapeMask ? static_cast<GlTexture *>(shapeMask)->name() : 0);
        glUniform1i(program.uniform("uShape"), 1);
        glActiveTexture(GL_TEXTURE0);
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    drawQuad(program, result.get(), bounds, false);
    glDisable(GL_BLEND);
    if (shaped) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
    }
    glScissor(oldScissor[0], oldScissor[1], oldScissor[2], oldScissor[3]);
    if (!wasScissored)
        glDisable(GL_SCISSOR_TEST);
}

} // namespace Lingmo::Render

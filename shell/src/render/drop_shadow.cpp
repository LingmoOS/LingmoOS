// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/drop_shadow.h"

#include <algorithm>
#include <cmath>

#include "render/gaussian_kernel.h"
#include "render/gl_texture.h"
#include "render_shaders.h"
#include "base/geometry.h"

namespace Lingmo::Render {

namespace {

// Skia's kMaxLinearBlurSigma is 4; twice that still keeps three sigma inside
// the kernel's reach and halves once less, so the stretched copy stays smooth.
constexpr float kMaxSigma = 8;

} // namespace

bool DropShadow::initialize()
{
    m_program = GlProgram::create(shaders::texture_vertex_shader, shaders::blur_fragment_shader);
    return m_program != 0;
}

std::unique_ptr<Texture> DropShadow::render(Renderer &renderer, const base::Size &size,
                                            const std::vector<ShadowLobe> &lobes,
                                            const std::function<void()> &paintCaster)
{
    if (size.isEmpty() || !m_program)
        return nullptr;
    // The caster's coverage, then each level the one before it halved.
    std::vector<std::unique_ptr<Texture>> levels(1);
    if (!renderer.renderToTexture(levels[0], base::Rect(base::Point(), size), paintCaster))
        return nullptr;

    struct Blurred {
        std::unique_ptr<Texture> texture;
        int factor = 1;
        base::Point offset;
        float opacity = 0;
    };
    std::vector<Blurred> blurred;
    for (const ShadowLobe &lobe : lobes) {
        if (lobe.opacity <= 0)
            continue;
        size_t level = 0;
        float sigma = std::max(lobe.sigma, 0.0f);
        while (sigma > kMaxSigma) {
            sigma /= 2;
            ++level;
        }
        while (levels.size() <= level) {
            Texture *source = levels.back().get();
            const base::Size half((source->size().width() + 1) / 2, (source->size().height() + 1) / 2);
            std::unique_ptr<Texture> reduced;
            // Every pixel samples the corner its four source pixels share,
            // which linear filtering averages.
            if (!renderer.renderToTexture(reduced, base::Rect(base::Point(), half), [&] {
                    glUseProgram(m_program);
                    glDisable(GL_BLEND);
                    setGaussianKernel(m_program, 0, 0, 0);
                    draw(source, base::RectF(base::PointF(), half), half,
                         {2.0 * half.width() / source->size().width(),
                          2.0 * half.height() / source->size().height()});
                }))
                return nullptr;
            levels.push_back(std::move(reduced));
        }

        Texture *source = levels[level].get();
        const base::Size reduced = source->size();
        // Truncated at three sigma, which is what the caller's margins allow for.
        const int reach = int(std::ceil(3 * sigma));
        const auto pass = [&](Texture *input, std::unique_ptr<Texture> &output, float stepX,
                              float stepY) {
            return renderer.renderToTexture(output, base::Rect(base::Point(), reduced), [&] {
                glUseProgram(m_program);
                glDisable(GL_BLEND);
                setGaussianKernel(m_program, sigma, stepX, stepY, reach);
                draw(input, base::RectF(base::PointF(), reduced), reduced);
            });
        };
        std::unique_ptr<Texture> horizontal;
        std::unique_ptr<Texture> vertical;
        if (!pass(source, horizontal, 1.0f / reduced.width(), 0)
            || !pass(horizontal.get(), vertical, 0, 1.0f / reduced.height()))
            return nullptr;
        blurred.push_back({std::move(vertical), 1 << level, lobe.offset, lobe.opacity});
    }

    std::unique_ptr<Texture> result;
    if (!renderer.renderToTexture(result, base::Rect(base::Point(), size), [&] {
            glUseProgram(m_program);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            setGaussianKernel(m_program, 0, 0, 0);
            for (const Blurred &lobe : blurred) {
                glUniform1f(m_program.uniform("uCenterWeight"), lobe.opacity);
                // Stretched back over the pixels it stands for. Rows run
                // bottom-up, so down the image is down GL's y as well.
                const base::SizeF covered = base::SizeF(lobe.texture->size()) * lobe.factor;
                draw(lobe.texture.get(), base::RectF(base::PointF(lobe.offset.x(), -lobe.offset.y()), covered),
                     size);
            }
            // The outer blur style: nothing of the shadow shows where the caster is.
            glUniform1f(m_program.uniform("uCenterWeight"), 1);
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
            draw(levels[0].get(), base::RectF(base::PointF(), size), size);
            glDisable(GL_BLEND);
        }))
        return nullptr;
    return result;
}

void DropShadow::draw(Texture *texture, const base::RectF &rect, const base::Size &viewport,
                      const base::PointF &extent) const
{
    const float x0 = float(rect.left() / viewport.width() * 2 - 1);
    const float x1 = float(rect.right() / viewport.width() * 2 - 1);
    const float y0 = float(rect.top() / viewport.height() * 2 - 1);
    const float y1 = float(rect.bottom() / viewport.height() * 2 - 1);
    const float vertices[8] = {x0, y0, x1, y0, x0, y1, x1, y1};
    const float s = float(extent.x());
    const float t = float(extent.y());
    const float coordinates[8] = {0, 0, s, 0, 0, t, s, t};
    const GLuint position = GlProgram::kPositionAttribute;
    const GLuint coord = GlProgram::kTexCoordAttribute;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, static_cast<GlTexture *>(texture)->name());
    glUniform1i(m_program.uniform("uTexture"), 0);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
    glEnableVertexAttribArray(position);
    glEnableVertexAttribArray(coord);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(coord);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Lingmo::Render

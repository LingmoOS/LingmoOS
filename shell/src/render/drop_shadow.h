// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "render/gl_program.h"
#include "render/renderer.h"
#include "base/geometry.h"

namespace Lingmo::Render {

// Renderer::renderShadow() the way Skia's GPU mask blur does it (GrBlurUtils:
// GaussianBlur, then the outer style's difference with the source mask): every
// lobe is a separable Gaussian of the caster's coverage, run on a copy halved
// until its sigma fits the linear-tap kernel and stretched back over the image.
class DropShadow
{
public:
    bool initialize();

    // With the renderer's raster scale at 1, so logical units are image pixels,
    // and no rounded clip set.
    std::unique_ptr<Texture> render(Renderer &renderer, const base::Size &size,
                                    const std::vector<ShadowLobe> &lobes,
                                    const std::function<void()> &paintCaster);

private:
    // Draws `texture` over `rect`, in pixels of a `viewport` sized pass with
    // rows bottom-up, as every pass here stores them; `extent` is the texture
    // coordinate the far corner samples.
    void draw(Texture *texture, const base::RectF &rect, const base::Size &viewport,
              const base::PointF &extent = {1, 1}) const;

    GlProgram m_program;
};

} // namespace Lingmo::Render

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once


#include "render/gl_program.h"
#include "base/geometry.h"

namespace Lingmo::Render {

// The farthest texel blur_fragment_shader reaches: fifteen linear taps of two.
inline constexpr int kGaussianMaxReach = 30;

// Sets blur_fragment_shader's kernel to a Gaussian of `sigma` texels, cut off
// `reach` texels out; `stepX` and `stepY` are one texel in texture
// coordinates. A sigma of zero copies. Samples are kept to the whole texture,
// which setGaussianClamp() narrows. `program` must be in use.
void setGaussianKernel(const GlProgram &program, float sigma, float stepX, float stepY,
                       int reach = kGaussianMaxReach);

// Keeps the samples of blur_fragment_shader, or of a shader declaring its
// uClamp, to `texels` of a texture of `size`, as GL_CLAMP_TO_EDGE keeps them
// to the whole texture. Rows count bottom-up, as GL lays them.
void setGaussianClamp(const GlProgram &program, const base::Rect &texels, const base::Size &size);

} // namespace Lingmo::Render

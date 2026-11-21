// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/gaussian_kernel.h"

#include <algorithm>
#include <cmath>
#include "base/geometry.h"

namespace Lingmo::Render {

void setGaussianKernel(const GlProgram &program, float sigma, float stepX, float stepY, int reach)
{
    reach = std::clamp(reach, 0, kGaussianMaxReach);
    // Beyond three and a half sigma the tail weighs under a thousandth, which
    // no 8-bit output shows; the taps out there would only cost fetches.
    if (sigma > 0)
        reach = std::min(reach, int(std::ceil(3.5f * sigma)));
    float weights[kGaussianMaxReach + 1] = {};
    float total = 0;
    for (int i = 0; i <= reach; ++i) {
        if (sigma > 0)
            weights[i] = std::exp(-float(i * i) / (2 * sigma * sigma));
        else
            weights[i] = i == 0 ? 1 : 0;
        total += weights[i] * (i == 0 ? 1 : 2);
    }
    for (float &weight : weights)
        weight /= total;
    // Linear filtering combines adjacent Gaussian taps without changing the kernel.
    float taps[kGaussianMaxReach] = {};
    int tapCount = 0;
    for (int i = 1; i < kGaussianMaxReach; i += 2) {
        const float weight = weights[i] + weights[i + 1];
        if (weight == 0)
            break;
        taps[2 * tapCount] = i + weights[i + 1] / weight;
        taps[2 * tapCount + 1] = weight;
        ++tapCount;
    }
    glUniform1f(program.uniform("uCenterWeight"), weights[0]);
    glUniform1i(program.uniform("uTapCount"), tapCount);
    if (tapCount > 0)
        glUniform2fv(program.uniform("uTaps"), tapCount, taps);
    glUniform2f(program.uniform("uStep"), stepX, stepY);
    glUniform4f(program.uniform("uClamp"), 0, 0, 1, 1);
}

void setGaussianClamp(const GlProgram &program, const base::Rect &texels, const base::Size &size)
{
    glUniform4f(program.uniform("uClamp"), (texels.left() + 0.5f) / size.width(),
                (texels.top() + 0.5f) / size.height(), (texels.right() - 0.5f) / size.width(),
                (texels.bottom() - 0.5f) / size.height());
}

} // namespace Lingmo::Render

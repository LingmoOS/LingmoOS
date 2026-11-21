// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

namespace Lingmo::Render {

// Gains for the red, green and blue channels of a whole output, applied to the
// encoded values the display is sent, after composition: night light's tint.
struct ColorTransform {
    float red = 1;
    float green = 1;
    float blue = 1;

    bool isIdentity() const { return red == 1 && green == 1 && blue == 1; }

    friend bool operator==(const ColorTransform &, const ColorTransform &) = default;
};

} // namespace Lingmo::Render

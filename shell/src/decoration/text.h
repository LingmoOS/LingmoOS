// Copyright 2012 The Chromium Authors
// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/gfx/render_text{,_harfbuzz}.cc
//           chromium/src/ui/gfx/font_fallback_linux.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_DECORATION_TEXT_H_
#define LINGMO_DECORATION_TEXT_H_

#include <memory>
#include <string>
#include <string_view>

#include "base/geometry.h"

#include "decoration/raster.h"

namespace Lingmo::Decoration {

// Single-line system-font layout; owned by the decoration theme on the compositor thread.
class TextRenderer
{
public:
    TextRenderer();
    ~TextRenderer();
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    base::Size textSize(std::string_view text, int pixelSize);
    // `rasterDensity` may be fractional: the text is rasterized at the size it
    // is shown at, not at a whole multiple scaled down.
    Image renderText(std::string_view text, int pixelSize, const render::Color& color,
                     double rasterDensity = 1);
    std::string elideText(std::string_view text, int pixelSize, int maxWidth);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Lingmo::Decoration

#endif // LINGMO_DECORATION_TEXT_H_

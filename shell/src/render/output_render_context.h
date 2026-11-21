// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/root_window_transformers.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: describe one pass over the shared logical scene
// without creating an Aura root window per output.

#pragma once

#include <cstdint>

#include <string>
#include "base/geometry.h"

namespace Lingmo::Render {

struct OutputRenderContext {
    std::string outputId;
    base::Rect logicalViewport;
    base::Size nativeTargetSize;
    double scale = 1;
    uint32_t transform = 0;
    uint64_t frame = 0;
    base::Rect nativeContentRect;
};

} // namespace Lingmo::Render

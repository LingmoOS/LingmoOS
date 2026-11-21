// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "compositor/output.h"
#include <string>

#include "base/geometry.h"

#include "base/signal.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::Compositor {

// What an ext_image_capture_source_v1 refers to: a whole output, or one
// window painted on its own. The owner keeps the fields current and emits
// changed() when the buffer constraints move or the source goes away.
class CaptureSource final
{
public:
    enum class Kind {
        Output,
        Window,
    };

    Kind kind = Kind::Output;
    Output output;
    // Window sources: the subtree to paint, its logical bounds in global
    // coordinates, the scale it is rasterized at, and the output whose
    // composition carries it.
    const scene::Node *node = nullptr;
    base::Rect bounds;
    double scale = 1;
    std::string outputName;
    bool active = true;

    base::Size pixelSize() const
    {
        if (kind == Kind::Output)
            return output.pixelSize;
        return base::Size(base::roundToInt(bounds.width() * scale),
                          base::roundToInt(bounds.height() * scale));
    }

    std::string renderedOn() const { return kind == Kind::Output ? output.name : outputName; }

    base::Signal<> changed;
};

} // namespace Lingmo::Compositor

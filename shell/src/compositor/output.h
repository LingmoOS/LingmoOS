// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>

#include "base/geometry.h"

#include "compositor/output_scale.h"
#include "render/presentation.h"

namespace Lingmo::Compositor {

// Metrics for the selected native mode; UI scale never changes pixelSize.
struct Output {
    enum class Subpixel {
        Unknown,
        None,
        HorizontalRgb,
        HorizontalBgr,
        VerticalRgb,
        VerticalBgr
    };

    enum class Transform {
        Normal,
        Rotate90,
        Rotate180,
        Rotate270,
    };

    base::Size orientedPixelSize() const
    {
        return transform == Transform::Rotate90 || transform == Transform::Rotate270
                ? base::Size(pixelSize.height(), pixelSize.width())
                : pixelSize;
    }

    base::Size logicalSize() const
    {
        return logicalExtent.isEmpty() ? scale.logicalSize(orientedPixelSize()) : logicalExtent;
    }

    base::Point position;
    // -1 until known, which is what wl_output reports for an unset size.
    base::Size physicalSizeMm {-1, -1};
    base::Size pixelSize {-1, -1};
    std::string make = "unknown";
    std::string model;
    int refreshMilliHz = 0;
    bool preferred = false;
    Subpixel subpixel = Subpixel::Unknown;
    OutputScale scale;
    std::shared_ptr<render::PresentationOutput> presentation =
            std::make_shared<render::PresentationOutput>();
    std::string name = "Lingmo-1";
    std::string description;
    Transform transform = Transform::Normal;
    double renderScale = 1.0;
    base::Rect nativeContentRect;
    base::Size logicalExtent;
    std::string displayId;
};
} // namespace Lingmo::Compositor

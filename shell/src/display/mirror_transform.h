// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/root_window_transformers.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: maps one logical mirror source through a destination's
// letterboxed native content rectangle without depending on Aura windows.

#pragma once

#include <optional>

#include "base/geometry.h"

#include "display/display_types.h"

namespace Lingmo::Display {

class MirrorTransform
{
public:
    MirrorTransform() = default;
    MirrorTransform(base::Rect logicalBounds, base::Rect nativeContent, DisplayRotation rotation);

    bool isValid() const;
    base::PointF logicalToNative(const base::PointF& point) const;
    std::optional<base::PointF> nativeToLogical(const base::PointF& point) const;

private:
    base::PointF rotate(const base::PointF& normalized) const;
    base::PointF unrotate(const base::PointF& normalized) const;

    base::Rect m_logicalBounds;
    base::Rect m_nativeContent;
    DisplayRotation m_rotation = DisplayRotation::Normal;
};

} // namespace Lingmo::Display

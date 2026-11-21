// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/root_window_transformers.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: one value object owns the global-logical/native
// forward and inverse transforms without Aura root windows.

#pragma once

#include <optional>

#include "base/geometry.h"
#include "compositor/output_scale.h"
#include "display/display_types.h"

namespace Lingmo::Display {

class DisplayTransform
{
public:
    DisplayTransform() = default;
    DisplayTransform(base::Point logicalOrigin, base::Size nativeSize,
                     compositor::OutputScale scale,
                     DisplayRotation rotation);

    bool isValid() const;
    base::Size nativeSize() const { return m_nativeSize; }
    base::Size rotatedNativeSize() const;
    base::Size logicalSize() const;
    base::Rect logicalBounds() const;
    compositor::OutputScale scale() const { return m_scale; }
    DisplayRotation rotation() const { return m_rotation; }

    bool containsGlobal(const base::PointF& point) const;
    base::PointF globalToNative(const base::PointF& point) const;
    base::PointF nativeToGlobal(const base::PointF& point) const;
    base::RectF globalToNative(const base::RectF& rect) const;
    base::RectF nativeToGlobal(const base::RectF& rect) const;

private:
    base::PointF rotatedToNative(const base::PointF& point) const;
    base::PointF nativeToRotated(const base::PointF& point) const;

    base::Point m_logicalOrigin;
    base::Size m_nativeSize;
    compositor::OutputScale m_scale;
    DisplayRotation m_rotation = DisplayRotation::Normal;
};

} // namespace Lingmo::Display

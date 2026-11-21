// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/root_window_transformers.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: point and rectangle mappings cover fractional
// scale, negative origins, and all compositor-supported rotations.

#include "display/display_transform.h"

#include <algorithm>
#include <array>


namespace Lingmo::Display {

namespace {

template<typename Mapper>
base::RectF mapRectangle(const base::RectF& rect, Mapper mapper)
{
    const std::array<base::PointF, 4> points {
        mapper(rect.topLeft()), mapper(rect.topRight()), mapper(rect.bottomLeft()),
        mapper(rect.bottomRight())};
    double left = points[0].x();
    double right = left;
    double top = points[0].y();
    double bottom = top;
    for (const base::PointF& point : points) {
        left = std::min(left, point.x());
        right = std::max(right, point.x());
        top = std::min(top, point.y());
        bottom = std::max(bottom, point.y());
    }
    return base::RectF::fromEdges(left, top, right, bottom);
}

} // namespace

DisplayTransform::DisplayTransform(base::Point logicalOrigin, base::Size nativeSize,
                                   compositor::OutputScale scale, DisplayRotation rotation)
    : m_logicalOrigin(logicalOrigin)
    , m_nativeSize(nativeSize)
    , m_scale(scale)
    , m_rotation(rotation)
{
}

bool DisplayTransform::isValid() const
{
    return m_nativeSize.width() > 0 && m_nativeSize.height() > 0;
}

base::Size DisplayTransform::rotatedNativeSize() const
{
    if (m_rotation == DisplayRotation::Rotate90 || m_rotation == DisplayRotation::Rotate270)
        return m_nativeSize.transposed();
    return m_nativeSize;
}

base::Size DisplayTransform::logicalSize() const
{
    return m_scale.logicalSize(rotatedNativeSize());
}

base::Rect DisplayTransform::logicalBounds() const
{
    return base::Rect(m_logicalOrigin, logicalSize());
}

bool DisplayTransform::containsGlobal(const base::PointF& point) const
{
    const base::Size extent = logicalSize();
    return point.x() >= m_logicalOrigin.x() && point.y() >= m_logicalOrigin.y() &&
        point.x() < m_logicalOrigin.x() + extent.width() &&
        point.y() < m_logicalOrigin.y() + extent.height();
}

base::PointF DisplayTransform::globalToNative(const base::PointF& point) const
{
    return rotatedToNative((point - base::PointF(m_logicalOrigin)) * m_scale.value());
}

base::PointF DisplayTransform::nativeToGlobal(const base::PointF& point) const
{
    return nativeToRotated(point) / m_scale.value() + base::PointF(m_logicalOrigin);
}

base::RectF DisplayTransform::globalToNative(const base::RectF& rect) const
{
    return mapRectangle(rect, [this](const base::PointF& point) { return globalToNative(point); });
}

base::RectF DisplayTransform::nativeToGlobal(const base::RectF& rect) const
{
    return mapRectangle(rect, [this](const base::PointF& point) { return nativeToGlobal(point); });
}

base::PointF DisplayTransform::rotatedToNative(const base::PointF& point) const
{
    switch (m_rotation) {
    case DisplayRotation::Normal:
        return point;
    case DisplayRotation::Rotate90:
        return base::PointF(point.y(), m_nativeSize.height() - point.x());
    case DisplayRotation::Rotate180:
        return base::PointF(m_nativeSize.width() - point.x(), m_nativeSize.height() - point.y());
    case DisplayRotation::Rotate270:
        return base::PointF(m_nativeSize.width() - point.y(), point.x());
    }
    return point;
}

base::PointF DisplayTransform::nativeToRotated(const base::PointF& point) const
{
    switch (m_rotation) {
    case DisplayRotation::Normal:
        return point;
    case DisplayRotation::Rotate90:
        return base::PointF(m_nativeSize.height() - point.y(), point.x());
    case DisplayRotation::Rotate180:
        return base::PointF(m_nativeSize.width() - point.x(), m_nativeSize.height() - point.y());
    case DisplayRotation::Rotate270:
        return base::PointF(point.y(), m_nativeSize.width() - point.x());
    }
    return point;
}

} // namespace Lingmo::Display

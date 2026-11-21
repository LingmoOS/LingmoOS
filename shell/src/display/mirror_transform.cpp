// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/root_window_transformers.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: normalized coordinates provide a reversible mapping
// for differently sized, rotated, and letterboxed mirror destinations.

#include "display/mirror_transform.h"

namespace Lingmo::Display {

MirrorTransform::MirrorTransform(base::Rect logicalBounds, base::Rect nativeContent,
                                 DisplayRotation rotation)
    : m_logicalBounds(logicalBounds)
    , m_nativeContent(nativeContent)
    , m_rotation(rotation)
{
}

bool MirrorTransform::isValid() const
{
    return !m_logicalBounds.isEmpty() && !m_nativeContent.isEmpty();
}

base::PointF MirrorTransform::rotate(const base::PointF& point) const
{
    switch (m_rotation) {
    case DisplayRotation::Normal:
        return point;
    case DisplayRotation::Rotate90:
        return {point.y(), 1.0 - point.x()};
    case DisplayRotation::Rotate180:
        return {1.0 - point.x(), 1.0 - point.y()};
    case DisplayRotation::Rotate270:
        return {1.0 - point.y(), point.x()};
    }
    return point;
}

base::PointF MirrorTransform::unrotate(const base::PointF& point) const
{
    switch (m_rotation) {
    case DisplayRotation::Normal:
        return point;
    case DisplayRotation::Rotate90:
        return {1.0 - point.y(), point.x()};
    case DisplayRotation::Rotate180:
        return {1.0 - point.x(), 1.0 - point.y()};
    case DisplayRotation::Rotate270:
        return {point.y(), 1.0 - point.x()};
    }
    return point;
}

base::PointF MirrorTransform::logicalToNative(const base::PointF& point) const
{
    if (!isValid())
        return {};
    base::PointF normalized((point.x() - m_logicalBounds.x()) / m_logicalBounds.width(),
                       (point.y() - m_logicalBounds.y()) / m_logicalBounds.height());
    normalized = rotate(normalized);
    return {m_nativeContent.x() + normalized.x() * m_nativeContent.width(),
            m_nativeContent.y() + normalized.y() * m_nativeContent.height()};
}

std::optional<base::PointF> MirrorTransform::nativeToLogical(const base::PointF& point) const
{
    if (!isValid() || point.x() < m_nativeContent.x() || point.y() < m_nativeContent.y() ||
        point.x() >= m_nativeContent.x() + m_nativeContent.width() ||
        point.y() >= m_nativeContent.y() + m_nativeContent.height()) {
        return std::nullopt;
    }
    base::PointF normalized((point.x() - m_nativeContent.x()) / m_nativeContent.width(),
                       (point.y() - m_nativeContent.y()) / m_nativeContent.height());
    normalized = unrotate(normalized);
    return base::PointF(m_logicalBounds.x() + normalized.x() * m_logicalBounds.width(),
                   m_logicalBounds.y() + normalized.y() * m_logicalBounds.height());
}

} // namespace Lingmo::Display

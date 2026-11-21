// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include "base/geometry.h"
#include "ui/geometry.h"

// Between the views' plain structs and the base classes. Kept out of
// ui/geometry.h: files using both namespaces would see two PointF.
namespace Lingmo::ui {

inline base::PointF toBase(const PointF& point)
{
    return {point.x, point.y};
}

inline base::SizeF toBase(const SizeF& size)
{
    return {size.width, size.height};
}

inline base::RectF toBase(const RectF& rect)
{
    return {rect.x, rect.y, rect.width, rect.height};
}

inline base::Rect toBase(const Rect& rect)
{
    return {rect.x, rect.y, rect.width, rect.height};
}

inline base::InsetsF toBase(const Margins& margins)
{
    return {margins.left, margins.top, margins.right, margins.bottom};
}

inline PointF fromBase(const base::PointF& point)
{
    return {point.x(), point.y()};
}

inline SizeF fromBase(const base::SizeF& size)
{
    return {size.width(), size.height()};
}

inline RectF fromBase(const base::RectF& rect)
{
    return {rect.x(), rect.y(), rect.width(), rect.height()};
}

inline Rect fromBase(const base::Rect& rect)
{
    return {rect.x(), rect.y(), rect.width(), rect.height()};
}

inline Margins fromBase(const base::InsetsF& insets)
{
    return {insets.left(), insets.top(), insets.right(), insets.bottom()};
}

} // namespace Lingmo::ui

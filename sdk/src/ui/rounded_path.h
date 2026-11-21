// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"

namespace Lingmo::ui {

// A rounded rect whose corners are circular arcs, or the continuous corners
// the compositor gives window silhouettes: a cubic superellipse meeting the
// straight edges with zero curvature, over 1.5 times the nominal radius. Keep
// it in step with FishUI's roundedpath.cpp and lingmo-aurora's
// rounded_shape.h.
SkPath roundedRectPath(const SkRect& rect, double radius, bool continuous);

} // namespace Lingmo::ui

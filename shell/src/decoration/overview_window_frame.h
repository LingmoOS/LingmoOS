// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_DECORATION_OVERVIEW_WINDOW_FRAME_H_
#define LINGMO_DECORATION_OVERVIEW_WINDOW_FRAME_H_

#include "scene/overview_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Decoration {

class Theme;

// Every overview window has the corners, the shadow and the outline of a
// window in its normal state, the shadow of the active one.
scene::OverviewWindowFrame overviewWindowFrame(const Theme *theme);

} // namespace Lingmo::Decoration

#endif // LINGMO_DECORATION_OVERVIEW_WINDOW_FRAME_H_

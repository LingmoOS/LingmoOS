// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <array>
#include <functional>
#include <vector>

#include "accelerators/hot_corner_settings.h"
#include "base/geometry.h"

namespace Lingmo::Input {
class Seat;
} // namespace Lingmo::Input

namespace Lingmo::Accelerators {

// Runs a corner's action the moment the pointer reaches it. Only the outer
// corners of the whole layout count, not where two outputs meet. A corner
// fires once per visit: it waits for the pointer to leave its neighbourhood
// before it can fire again. Fullscreen windows do not stop it; a drag does,
// and so do a pointer locked by a client and a locked session, whose motion
// never reaches here.
class HotCornerController
{
public:
    // How far the pointer has to go from a corner, in logical pixels on either
    // axis, before the corner fires again.
    static constexpr int kRearmDistance = 8;

    using Perform = std::function<bool(AcceleratorAction)>;

    // `seat` gives the held modifiers and the pointer lock; it may be null in
    // tests, which then hold none.
    HotCornerController(input::Seat *seat, Perform perform);

    // The outputs' logical bounds.
    void setOutputs(const std::vector<base::Rect> &outputs);
    void setBinding(HotCorner corner, const HotCornerBinding &binding);

    // Every pointer move after clamping to the layout. A drag of any kind
    // (a held button, a window move, a drag and drop) uses the visit up
    // without running the action.
    void onPointerMoved(const base::PointF &position, bool dragging = false);

    // The outer corners, for tests.
    std::vector<std::pair<HotCorner, base::Point>> corners() const;

    uint32_t heldModifiers() const;

private:
    struct Corner {
        HotCorner corner;
        base::Point point;
        bool armed = true;
    };

    input::Seat *m_seat = nullptr;
    Perform m_perform;
    std::vector<Corner> m_corners;
    std::array<HotCornerBinding, 4> m_bindings;
    base::PointF m_lastPosition {-1e9, -1e9};
};

} // namespace Lingmo::Accelerators

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "accelerators/hot_corner_controller.h"

#include <cmath>
#include <cstdlib>

#include <xkbcommon/xkbcommon.h>

#include "accelerators/accelerator.h"
#include "input/pointer_constraints.h"
#include "input/seat.h"
#include "input/xkb_tracker.h"

namespace Lingmo::Accelerators {
namespace {

bool onAnyOutput(const std::vector<base::Rect> &outputs, const base::Point &point)
{
    for (const base::Rect &output : outputs) {
        if (output.contains(point))
            return true;
    }
    return false;
}

} // namespace

HotCornerController::HotCornerController(input::Seat *seat, Perform perform)
    : m_seat(seat)
    , m_perform(std::move(perform))
{
}

void HotCornerController::setOutputs(const std::vector<base::Rect> &outputs)
{
    m_corners.clear();
    const base::Point pointer(int(std::floor(m_lastPosition.x())),
                              int(std::floor(m_lastPosition.y())));
    for (const base::Rect &output : outputs) {
        // Each corner with the direction out of the output from it.
        const std::array<std::tuple<HotCorner, base::Point, base::Point>, 4> candidates = {{
                {HotCorner::TopLeft, output.topLeft(), base::Point(-1, -1)},
                {HotCorner::TopRight, base::Point(output.right() - 1, output.y()),
                 base::Point(1, -1)},
                {HotCorner::BottomLeft, base::Point(output.x(), output.bottom() - 1),
                 base::Point(-1, 1)},
                {HotCorner::BottomRight, output.bottomRight() - base::Point(1, 1),
                 base::Point(1, 1)},
        }};
        for (const auto &[corner, point, out] : candidates) {
            // Where two outputs meet the pointer goes on across: not a corner.
            if (onAnyOutput(outputs, point + base::Point(out.x(), 0))
                || onAnyOutput(outputs, point + base::Point(0, out.y()))
                || onAnyOutput(outputs, point + out))
                continue;
            // One the pointer already rests on waits for it to leave.
            m_corners.push_back({corner, point, point != pointer});
        }
    }
}

void HotCornerController::setBinding(HotCorner corner, const HotCornerBinding &binding)
{
    m_bindings[size_t(corner)] = binding;
}

void HotCornerController::onPointerMoved(const base::PointF &position, bool dragging)
{
    m_lastPosition = position;
    const base::Point pointer(int(std::floor(position.x())), int(std::floor(position.y())));
    for (Corner &corner : m_corners) {
        if (!corner.armed) {
            if (std::abs(pointer.x() - corner.point.x()) > kRearmDistance
                || std::abs(pointer.y() - corner.point.y()) > kRearmDistance)
                corner.armed = true;
            continue;
        }
        if (pointer != corner.point)
            continue;
        corner.armed = false;
        if (dragging || (m_seat && m_seat->pointerConstraints()->isLocked()))
            continue;
        const HotCornerBinding &binding = m_bindings[size_t(corner.corner)];
        if (!binding.action)
            continue;
        if (binding.modifiers != NoModifier && heldModifiers() != binding.modifiers)
            continue;
        m_perform(*binding.action);
    }
}

std::vector<std::pair<HotCorner, base::Point>> HotCornerController::corners() const
{
    std::vector<std::pair<HotCorner, base::Point>> result;
    for (const Corner &corner : m_corners)
        result.emplace_back(corner.corner, corner.point);
    return result;
}

uint32_t HotCornerController::heldModifiers() const
{
    if (!m_seat || !m_seat->xkbTracker())
        return NoModifier;
    const input::XkbTracker *xkb = m_seat->xkbTracker();
    uint32_t modifiers = NoModifier;
    if (xkb->modifierPressed(XKB_MOD_NAME_SHIFT))
        modifiers |= ShiftDown;
    if (xkb->modifierPressed(XKB_MOD_NAME_CTRL))
        modifiers |= ControlDown;
    if (xkb->modifierPressed(XKB_MOD_NAME_ALT))
        modifiers |= AltDown;
    if (xkb->modifierPressed(XKB_MOD_NAME_LOGO))
        modifiers |= MetaDown;
    return modifiers;
}

} // namespace Lingmo::Accelerators

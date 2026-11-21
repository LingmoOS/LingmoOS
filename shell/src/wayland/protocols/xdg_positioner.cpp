// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_positioner.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wayland/protocols/xdg_positioner.h"

#include <xdg-shell-server-protocol.h>

#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <utility>

namespace Lingmo::Wayland {

namespace {

using Direction = WaylandPositioner::Direction;

std::pair<Direction, Direction> decomposeAnchor(uint32_t anchor)
{
    switch (anchor) {
    default:
    case XDG_POSITIONER_ANCHOR_NONE:
        return {Direction::kNeutral, Direction::kNeutral};
    case XDG_POSITIONER_ANCHOR_TOP:
        return {Direction::kNeutral, Direction::kNegative};
    case XDG_POSITIONER_ANCHOR_BOTTOM:
        return {Direction::kNeutral, Direction::kPositive};
    case XDG_POSITIONER_ANCHOR_LEFT:
        return {Direction::kNegative, Direction::kNeutral};
    case XDG_POSITIONER_ANCHOR_RIGHT:
        return {Direction::kPositive, Direction::kNeutral};
    case XDG_POSITIONER_ANCHOR_TOP_LEFT:
        return {Direction::kNegative, Direction::kNegative};
    case XDG_POSITIONER_ANCHOR_BOTTOM_LEFT:
        return {Direction::kNegative, Direction::kPositive};
    case XDG_POSITIONER_ANCHOR_TOP_RIGHT:
        return {Direction::kPositive, Direction::kNegative};
    case XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT:
        return {Direction::kPositive, Direction::kPositive};
    }
}

std::pair<Direction, Direction> decomposeGravity(uint32_t gravity)
{
    switch (gravity) {
    default:
    case XDG_POSITIONER_GRAVITY_NONE:
        return {Direction::kNeutral, Direction::kNeutral};
    case XDG_POSITIONER_GRAVITY_TOP:
        return {Direction::kNeutral, Direction::kNegative};
    case XDG_POSITIONER_GRAVITY_BOTTOM:
        return {Direction::kNeutral, Direction::kPositive};
    case XDG_POSITIONER_GRAVITY_LEFT:
        return {Direction::kNegative, Direction::kNeutral};
    case XDG_POSITIONER_GRAVITY_RIGHT:
        return {Direction::kPositive, Direction::kNeutral};
    case XDG_POSITIONER_GRAVITY_TOP_LEFT:
        return {Direction::kNegative, Direction::kNegative};
    case XDG_POSITIONER_GRAVITY_BOTTOM_LEFT:
        return {Direction::kNegative, Direction::kPositive};
    case XDG_POSITIONER_GRAVITY_TOP_RIGHT:
        return {Direction::kPositive, Direction::kNegative};
    case XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT:
        return {Direction::kPositive, Direction::kPositive};
    }
}

Direction flip(Direction direction)
{
    return static_cast<Direction>(-direction);
}

// The possible/actual positioner adjustments for this window.
struct ConstraintAdjustment {
    bool flip;
    bool slide;
    bool resize;

    bool allowsAll() const { return flip && slide && resize; }
};

ConstraintAdjustment maskToConstraintAdjustment(uint32_t field, uint32_t flipMask,
                                                uint32_t slideMask, uint32_t resizeMask)
{
    return {!!(field & flipMask), !!(field & slideMask), !!(field & resizeMask)};
}

// A 1-dimensional projection of a range, used to solve the positioning problem
// one axis at a time.
struct Range1D {
    int32_t start;
    int32_t end;

    Range1D transposed(int32_t offset) const { return {start + offset, end + offset}; }

    int32_t center() const { return std::midpoint(start, end); }
};

// The range position that results from using exactly the adjustments in
// `adjustments`.
Range1D calculate(const ConstraintAdjustment& adjustments, int32_t workSize, Range1D anchorRange,
                  int32_t size, int32_t offset, Direction anchor, Direction gravity)
{
    if (adjustments.flip) {
        return calculate({/*flip=*/false, adjustments.slide, adjustments.resize}, workSize,
                         anchorRange, size, -offset, flip(anchor), flip(gravity));
    }
    if (adjustments.resize) {
        const Range1D unresized =
            calculate({/*flip=*/false, adjustments.slide, /*resize=*/false}, workSize, anchorRange,
                      size, offset, anchor, gravity);
        return {std::max(unresized.start, 0), std::min(unresized.end, workSize)};
    }
    if (adjustments.slide) {
        // Either the slide unconstrains the window, or the window is
        // constrained in the positive direction.
        Range1D unslid = calculate({/*flip=*/false, /*slide=*/false, /*resize=*/false}, workSize,
                                   anchorRange, size, offset, anchor, gravity);
        if (unslid.end > workSize)
            unslid = unslid.transposed(workSize - unslid.end);
        if (unslid.start < 0)
            return unslid.transposed(-unslid.start);
        return unslid;
    }

    int32_t start = offset;
    switch (anchor) {
    case Direction::kNegative:
        start += anchorRange.start;
        break;
    case Direction::kNeutral:
        start += anchorRange.center();
        break;
    case Direction::kPositive:
        start += anchorRange.end;
        break;
    }

    switch (gravity) {
    case Direction::kNegative:
        start -= size;
        break;
    case Direction::kNeutral:
        start -= size / 2;
        break;
    case Direction::kPositive:
        break;
    }
    return {start, start + size};
}

struct IntermediateAdjustmentResult {
    // Statistics for comparing two placements.
    struct Stats {
        // When false, this result is chosen only if it is the sole
        // unconstrained option.
        bool preferred;
        bool constrained;
        int32_t visibility;
    } stats;
    Range1D position;
    ConstraintAdjustment adjustment;
};

// Which adjustments - subject to being a subset of the allowed ones - give the
// best range position. This is a 1-dimensional projection of the
// window-positioning problem.
std::pair<Range1D, ConstraintAdjustment> determineBestConstraintAdjustment(
    const Range1D& workArea, const Range1D& anchorRange, int32_t size, int32_t offset,
    Direction anchor, Direction gravity, const ConstraintAdjustment& validAdjustments,
    bool avoidOcclusion)
{
    if (workArea.start != 0) {
        const int32_t shift = -workArea.start;
        const std::pair<Range1D, ConstraintAdjustment> shifted = determineBestConstraintAdjustment(
            workArea.transposed(shift), anchorRange.transposed(shift), size, offset, anchor,
            gravity, validAdjustments, avoidOcclusion);
        return {shifted.first.transposed(-shift), shifted.second};
    }

    // Cycle through the available combinations of adjustments and choose the
    // first one that maximizes the amount of the window visible on screen.
    // When several tie, the earlier one wins, so the preference order is
    // flip > slide > resize as the positioner specification requires.
    IntermediateAdjustmentResult best {{/*preferred=*/false, /*constrained=*/true,
                                        /*visibility=*/0},
                                       /*position=*/{0, 0},
                                       /*adjustment=*/ConstraintAdjustment {}};
    bool foundSolution = false;
    for (uint32_t adjustmentBitField = 0; adjustmentBitField < 8; ++adjustmentBitField) {
        const ConstraintAdjustment adjustment =
            maskToConstraintAdjustment(adjustmentBitField, /*flipMask=*/1, /*slideMask=*/2,
                                       /*resizeMask=*/4);
        if ((adjustment.flip && !validAdjustments.flip)
            || (adjustment.slide && !validAdjustments.slide)
            || (adjustment.resize && !validAdjustments.resize)) {
            continue;
        }

        // Sliding can occlude the parent menu, so prefer any other acceptable
        // placement over it.
        const bool possibleOcclusion = avoidOcclusion && adjustment.slide;

        const Range1D position =
            calculate(adjustment, workArea.end, anchorRange, size, offset, anchor, gravity);
        const bool constrained = position.start < 0 || position.end > workArea.end;
        const int32_t visibility =
            std::abs(std::min(position.end, workArea.end) - std::max(position.start, 0));

        const bool preferred = !possibleOcclusion && !constrained;
        bool isBetter = false;
        if (preferred) {
            if (!best.stats.preferred || visibility > best.stats.visibility)
                isBetter = true;
        } else if (!constrained && best.stats.constrained) {
            isBetter = true;
        }

        if (isBetter) {
            foundSolution = true;
            best = IntermediateAdjustmentResult {
                {preferred, constrained, visibility}, position, adjustment};
        }
    }

    // With no solution, allow every transformation. The default is not usable
    // as a fallback because it has a zero dimension.
    if (!foundSolution && !validAdjustments.allowsAll()) {
        const ConstraintAdjustment allowAll {/*flip=*/true, /*slide=*/true, /*resize=*/true};
        return determineBestConstraintAdjustment(workArea, anchorRange, size, offset, anchor,
                                                 gravity, allowAll, avoidOcclusion);
    }

    return {best.position, best.adjustment};
}

} // namespace

void WaylandPositioner::setAnchor(uint32_t anchor)
{
    std::tie(m_anchorX, m_anchorY) = decomposeAnchor(anchor);
}

void WaylandPositioner::setGravity(uint32_t gravity)
{
    std::tie(m_gravityX, m_gravityY) = decomposeGravity(gravity);
}

WaylandPositioner::Result WaylandPositioner::calculateBounds(const base::Rect& workArea,
                                                             const base::Point& anchorOffset) const
{
    WaylandPositioner moved = *this;
    moved.m_anchorRect.translate(anchorOffset);
    return moved.calculateBounds(workArea);
}

WaylandPositioner::Result WaylandPositioner::calculateBounds(const base::Rect& workArea) const
{
    const ConstraintAdjustment adjustmentsX = maskToConstraintAdjustment(
        m_adjustment, XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X,
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X,
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X);
    const ConstraintAdjustment adjustmentsY = maskToConstraintAdjustment(
        m_adjustment, XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y,
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y,
        XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y);

    // Some adjustments are preferred over others when the orthogonal
    // anchor+gravity means a slide could occlude the anchor rect, unless it is
    // occluded already. This stops the common case of a dropdown menu covering
    // the header it dropped from. Best effort: if nothing fits inside the work
    // area, the parent may still end up occluded.
    const bool xOccluded = !(m_anchorX == m_gravityX && m_anchorX != kNeutral);
    const bool yOccluded = !(m_anchorY == m_gravityY && m_anchorY != kNeutral);
    const bool avoidYOcclusion = xOccluded && !yOccluded;
    const bool avoidXOcclusion = yOccluded && !xOccluded;

    const std::pair<Range1D, ConstraintAdjustment> x = determineBestConstraintAdjustment(
        {workArea.x(), workArea.x() + workArea.width()},
        {m_anchorRect.x(), m_anchorRect.x() + m_anchorRect.width()}, m_size.width(), m_offset.x(),
        m_anchorX, m_gravityX, adjustmentsX, avoidXOcclusion);
    const std::pair<Range1D, ConstraintAdjustment> y = determineBestConstraintAdjustment(
        {workArea.y(), workArea.y() + workArea.height()},
        {m_anchorRect.y(), m_anchorRect.y() + m_anchorRect.height()}, m_size.height(),
        m_offset.y(), m_anchorY, m_gravityY, adjustmentsY, avoidYOcclusion);

    return {base::Point(x.first.start, y.first.start),
            base::Size(std::max(1, x.first.end - x.first.start),
                       std::max(1, y.first.end - y.first.start))};
}

} // namespace Lingmo::Wayland

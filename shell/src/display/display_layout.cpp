// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/display_layout.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: resolve a bounded layout graph, including primary
// rerooting and minimum shared-edge normalization, without Ash display state.

#include "display/display_layout.h"

#include <algorithm>
#include <cstdlib>
#include <deque>

namespace Lingmo::Display {

namespace {

DisplayLayout::Result failure(std::string message, std::vector<DisplayId> displays = {})
{
    DisplayLayout::Result result;
    result.error = {DisplayErrorCode::InvalidLayout, std::move(message), std::move(displays)};
    return result;
}

LayoutEdge opposite(LayoutEdge edge)
{
    switch (edge) {
    case LayoutEdge::Top:
        return LayoutEdge::Bottom;
    case LayoutEdge::Right:
        return LayoutEdge::Left;
    case LayoutEdge::Bottom:
        return LayoutEdge::Top;
    case LayoutEdge::Left:
        return LayoutEdge::Right;
    }
    return LayoutEdge::Right;
}

int normalizedOffset(int offset, LayoutEdge edge, const base::Size& child, const base::Size& parent,
                     int minimumSharedEdge)
{
    const bool horizontalEdge = edge == LayoutEdge::Top || edge == LayoutEdge::Bottom;
    const int childExtent = horizontalEdge ? child.width() : child.height();
    const int parentExtent = horizontalEdge ? parent.width() : parent.height();
    const int shared = std::min({minimumSharedEdge, childExtent,
                                 parentExtent});
    return std::clamp(offset, -childExtent + shared, parentExtent - shared);
}

base::Point childOrigin(const base::Rect& parent, const base::Size& childSize, LayoutEdge edge,
                        int offset)
{
    switch (edge) {
    case LayoutEdge::Top:
        return base::Point(parent.x() + offset, parent.y() - childSize.height());
    case LayoutEdge::Right:
        return base::Point(parent.x() + parent.width(), parent.y() + offset);
    case LayoutEdge::Bottom:
        return base::Point(parent.x() + offset, parent.y() + parent.height());
    case LayoutEdge::Left:
        return base::Point(parent.x() - childSize.width(), parent.y() + offset);
    }
    return parent.topLeft();
}

} // namespace

DisplayLayout::Result DisplayLayout::resolve(const DisplayId& primary,
                                             const std::map<DisplayId, base::Size>& displaySizes,
                                             const std::vector<DisplayPlacement>& placements,
                                             int minimumSharedEdge)
{
    if (minimumSharedEdge < 0)
        return failure("The minimum shared edge cannot be negative");
    if (displaySizes.empty())
        return failure("At least one enabled display is required");
    if (displaySizes.size() > maximumDisplayCount)
        return failure("The layout contains too many displays");
    if (!primary.isValid() || !displaySizes.contains(primary))
        return failure("The primary display is missing", {primary});

    for (const auto& [id, size] : displaySizes) {
        if (!id.isValid() || size.width() <= 0 || size.height() <= 0)
            return failure("A display has an invalid identity or size", {id});
    }
    if (placements.size() != displaySizes.size() - 1)
        return failure(
                "The placement graph must contain one edge per non-primary display");

    std::vector<DisplayPlacement> normalized;
    std::map<DisplayId, std::vector<int>> adjacency;
    normalized.reserve(placements.size());
    for (const DisplayPlacement& requested : placements) {
        if (!displaySizes.contains(requested.display) || !displaySizes.contains(requested.parent))
            return failure("A placement refers to an unknown display",
                           {requested.display, requested.parent});
        if (requested.display == requested.parent) {
            return failure("A display cannot be placed relative to itself",
                           {requested.display});
        }
        if (std::abs(int64_t(requested.offset)) > maximumCoordinate)
            return failure("A placement offset is outside the supported range",
                           {requested.display});

        DisplayPlacement placement = requested;
        placement.offset = normalizedOffset(placement.offset, placement.edge,
                                            displaySizes.at(placement.display),
                                            displaySizes.at(placement.parent), minimumSharedEdge);
        const int index = int(normalized.size());
        normalized.push_back(placement);
        adjacency[placement.display].push_back(index);
        adjacency[placement.parent].push_back(index);
    }

    Result result;
    result.bounds.insert_or_assign(primary, base::Rect(displaySizes.at(primary)));
    std::deque<DisplayId> pending;
    pending.push_back(primary);
    while (!pending.empty()) {
        const DisplayId current = pending.front();
        pending.pop_front();
        const auto edges = adjacency.find(current);
        if (edges == adjacency.end())
            continue;
        for (const int index : edges->second) {
            const DisplayPlacement& edge = normalized[size_t(index)];
            const bool forward = edge.parent == current;
            const DisplayId next = forward ? edge.display : edge.parent;
            if (result.bounds.contains(next))
                continue;

            const LayoutEdge direction = forward ? edge.edge : opposite(edge.edge);
            const int offset = forward ? edge.offset : -edge.offset;
            const base::Size nextSize = displaySizes.at(next);
            const base::Point origin =
                    childOrigin(result.bounds.at(current), nextSize, direction, offset);
            result.bounds.insert_or_assign(next, base::Rect(origin, nextSize));
            pending.push_back(next);
        }
    }

    if (result.bounds.size() != displaySizes.size())
        return failure("The placement graph is disconnected or cyclic");

    std::vector<DisplayId> ids;
    for (const auto& [id, bounds] : result.bounds)
        ids.push_back(id);
    for (size_t i = 0; i < ids.size(); ++i) {
        const base::Rect bounds = result.bounds.at(ids[i]);
        if (std::abs(int64_t(bounds.x())) > maximumCoordinate ||
            std::abs(int64_t(bounds.y())) > maximumCoordinate ||
            std::abs(int64_t(bounds.x()) + bounds.width()) > maximumCoordinate ||
            std::abs(int64_t(bounds.y()) + bounds.height()) > maximumCoordinate) {
            return failure("The resolved layout is outside the supported range",
                           {ids[i]});
        }
        for (size_t j = i + 1; j < ids.size(); ++j) {
            if (bounds.intersects(result.bounds.at(ids[j])))
                return failure("Displays overlap after resolving the layout",
                               {ids[i], ids[j]});
        }
    }

    result.succeeded = true;
    result.placements = std::move(normalized);
    return result;
}

} // namespace Lingmo::Display

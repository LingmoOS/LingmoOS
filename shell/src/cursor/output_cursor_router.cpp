// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/extended_mouse_warp_controller.cc
//           chromium/src/ash/display/root_window_transformers.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: route cached cursor pixels to every matching physical
// mirror sink and translate the global position for each destination.

#include "cursor/output_cursor_router.h"

#include <algorithm>

namespace Lingmo::Cursor {

static bool contains(const base::Rect &bounds, const base::PointF &position)
{
    return bounds.contains(position.toFlooredPoint());
}

void OutputCursorRouter::setTargets(std::vector<Target> targets)
{
    const bool visible = !m_activeTargets.empty();
    hideCursor();
    m_targets = std::move(targets);
    m_activeTargets.clear();
    if (visible)
        moveCursor(m_position);
}

bool OutputCursorRouter::upload(int index)
{
    if (index < 0 || index >= static_cast<int>(m_targets.size()) || !m_targets[index].sink ||
        m_pixels.empty()) {
        return false;
    }
    m_image.pixels = m_pixels.data();
    return m_targets[index].sink->setCursorImage(m_image);
}

bool OutputCursorRouter::setCursorImage(const render::CursorImage& image)
{
    if (!image.pixels || image.size.isEmpty() || image.stride <= 0)
        return false;
    m_pixels.assign(image.pixels, image.pixels + image.stride * image.size.height());
    m_image = image;
    m_image.pixels = m_pixels.data();
    std::vector<int> targets;
    for (int index = 0; index < static_cast<int>(m_targets.size()); ++index)
        if (contains(m_targets[index].logicalBounds, m_position))
            targets.push_back(index);
    // Between outputs moveCursor keeps the cursor where it is, so the new
    // image goes there too, as Ozone's DrmCursor::SetCursor keeps its window.
    if (targets.empty())
        targets = m_activeTargets;
    if (targets.empty() && !m_targets.empty())
        targets.push_back(0);
    std::vector<int> uploaded;
    for (int target : targets)
        if (upload(target))
            uploaded.push_back(target);
    for (int target : m_activeTargets)
        if (std::find(uploaded.begin(), uploaded.end(), target) == uploaded.end() &&
            target >= 0 && target < static_cast<int>(m_targets.size()) && m_targets[target].sink)
            m_targets[target].sink->hideCursor();
    // Replaced, not appended: an animated cursor sets an image every frame.
    m_activeTargets = std::move(uploaded);
    return !m_activeTargets.empty();
}

void OutputCursorRouter::hideCursor()
{
    for (int target : m_activeTargets)
        if (target >= 0 && target < static_cast<int>(m_targets.size()) && m_targets[target].sink)
            m_targets[target].sink->hideCursor();
    m_activeTargets.clear();
}

void OutputCursorRouter::moveCursor(const base::PointF& position)
{
    m_position = position;
    std::vector<int> targets;
    for (int index = 0; index < static_cast<int>(m_targets.size()); ++index) {
        if (contains(m_targets[index].logicalBounds, position))
            targets.push_back(index);
    }
    if (targets.empty())
        return;
    if (targets != m_activeTargets) {
        hideCursor();
        for (int target : targets)
            if (upload(target))
                m_activeTargets.push_back(target);
    }
    for (int target : m_activeTargets)
        m_targets[target].sink->moveCursor(
            m_targets[target].mirrorTransform.isValid()
                ? m_targets[target].mirrorTransform.logicalToNative(position)
                : position - base::PointF(m_targets[target].logicalBounds.topLeft()));
}

} // namespace Lingmo::Cursor

// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/extended_mouse_warp_controller.cc
//           chromium/src/ash/display/root_window_transformers.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: one logical cursor selects output-local KMS sinks,
// including every overlapping physical destination of a mirror group.

#pragma once

#include <cstdint>
#include <vector>

#include "base/geometry.h"
#include "display/mirror_transform.h"
#include "render/cursor_sink.h"

namespace Lingmo::Cursor {

class OutputCursorRouter final : public render::CursorSink
{
public:
    struct Target {
        // Where the output sits on the logical desktop; the mirror transform
        // takes a point there to the output's own pixels.
        base::Rect logicalBounds;
        render::CursorSink *sink = nullptr;
        display::MirrorTransform mirrorTransform;
    };

    void setTargets(std::vector<Target> targets);

    void refresh() { setTargets(m_targets); }

    bool setCursorImage(const render::CursorImage &image) override;
    void hideCursor() override;
    void moveCursor(const base::PointF &position) override;

    int activeTarget() const { return m_activeTargets.empty() ? -1 : m_activeTargets.front(); }

private:
    bool upload(int index);

    std::vector<Target> m_targets;
    std::vector<uint8_t> m_pixels;
    render::CursorImage m_image;
    base::PointF m_position;
    std::vector<int> m_activeTargets;
};

} // namespace Lingmo::Cursor

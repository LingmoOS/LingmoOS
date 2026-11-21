// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once


#include "render/rounded_shape.h"
#include "scene/node.h"
#include "base/geometry.h"
#include "base/transform.h"
#include "ui/path.h"

namespace Lingmo::Scene {

enum class WindowVisualMode {
    ServerFrame,
    ServerExterior,
    PreserveClient
};

struct WindowVisualState {
    WindowVisualMode mode = WindowVisualMode::PreserveClient;
    render::RoundedShape shape;
    // Empty unless the outline follows a client-described body instead of shape.
    render::RoundedShape outlineShape;
    // A body of any outline; outlineShape then only bounds it.
    ui::Path outlinePath;
    // Where the client surface drawing the body sits; the surface is snapped to
    // physical pixels, so the clip and outline have to move with it.
    base::Point surfaceOrigin;
    render::Color outlineColor;
    render::Color highlightColor {0, 0, 0, 0};
    double outlineWidthPx = 0;
    // Black over the body while a modal transient blocks the window; 0 is none.
    float dimOpacity = 0;
    bool clipContent = false;
    bool drawOutline = false;

    const render::RoundedShape &outlined() const
    {
        return outlineShape.bounds.isEmpty() ? shape : outlineShape;
    }
};

// Dark outline one stroke outside the shape, highlight on its outermost pixels.
void paintWindowOutline(render::Renderer &renderer, const render::RoundedShape &shape,
                        const WindowVisualState &state, float opacity = 1);

// The same around state.outlinePath, rasterized as it is and then placed by
// `transform`, so an animation stretches it instead of rasterizing every frame.
void paintShapeOutline(render::Renderer &renderer, const WindowVisualState &state,
                       const base::AffineTransform &transform, float opacity = 1);

// How far the client body moves with its surface onto physical pixels, for a
// window whose node sits at `position`.
base::PointF clientBodySnap(const render::Renderer &renderer, const WindowVisualState &state,
                       const base::PointF &position);

// Presentation values only; closing snapshots never retain a Window pointer.
class WindowVisualNode : public Node
{
public:
    WindowVisualNode()
        : Node("window-visual")
    {
    }

    void setState(const WindowVisualState &state)
    {
        if (state.dimOpacity != m_state.dimOpacity) {
            // Fades from wherever a fade in flight got to, so a dialog closed
            // mid-fade reverses instead of jumping.
            m_dimFrom = m_dimPresented;
            m_dimStarted = -1;
            m_dimFading = true;
        }
        m_state = state;
        setGeometry(state.shape.bounds.toEnclosingRect());
        // Shape coordinates are relative to the window root, not this node.
        setPosition(base::Point());
        notifyChanged();
    }

    base::Rect absolutePaintBounds() const override
    {
        base::RectF bounds = m_state.shape.bounds;
        // The outline is at most one physical pixel wide, so one logical pixel covers it.
        if (m_state.drawOutline)
            bounds |= m_state.outlined().bounds.adjusted(-1, -1, 1, 1);
        return bounds.translated(absolutePosition()).toEnclosingRect();
    }

    const WindowVisualState &state() const { return m_state; }

    bool requiresFullRepaint() const override { return false; }

    bool paintsSubtree() const override
    {
        return m_state.clipContent || m_state.drawOutline || m_state.dimOpacity > 0 || m_dimFading;
    }

    // Input follows client regions, including resize margins outside the visual clip.
    void render(render::Renderer &renderer) const override;

private:
    // The dim presented by this frame; steps on the renderer's frame time like
    // WindowAnimation, so it is settled in render() rather than by a timer.
    float presentedDim(const render::Renderer &renderer) const;

    WindowVisualState m_state;
    mutable float m_dimFrom = 0;
    mutable float m_dimPresented = 0;
    mutable double m_dimStarted = -1;
    mutable bool m_dimFading = false;
};

} // namespace Lingmo::Scene

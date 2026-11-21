// Copyright 2012 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/compositor_extra/shadow.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "decoration/shadow_node.h"

#include <algorithm>
#include <cmath>

#include "compositor/surface.h"
#include "ui/geometry_conversions.h"
#include "decoration/decoration_theme.h"
#include "decoration/shadow.h"
#include "wm/window.h"
#include "wm/window_visual_controller.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Decoration {

namespace {

bool operator==(const Corners &a, const Corners &b)
{
    return a.topLeft == b.topLeft && a.topRight == b.topRight && a.bottomLeft == b.bottomLeft
            && a.bottomRight == b.bottomRight;
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// ShadowNode:

ShadowNode::ShadowNode(const Theme *theme)
    : scene::Node("shadow")
    , m_theme(theme)
{
}

void ShadowNode::update(const base::Rect &content, wm::ShadowElevation elevation,
                        const Corners &corners, int radius, const ui::Path &silhouette)
{
    if (content == m_content && elevation == m_requested && corners == m_corners
        && radius == m_radius && silhouette == m_silhouette)
        return;

    const ShadowGeometry geometry(content.size(), radius, m_theme->metrics().shadowFor(elevation),
                                  1, corners, silhouette);
    // An elevation with no style, or a window too small for one, draws nothing.
    const bool visible =
            elevation != wm::ShadowElevation::None && !geometry.paintBounds().isEmpty();

    m_content = content;
    m_corners = corners;
    m_radius = radius;
    m_silhouette = silhouette;
    m_requested = elevation;
    m_elevation = visible ? elevation : wm::ShadowElevation::None;
    m_margins = visible ? geometry.margins() : base::Insets();

    // Hide before the geometry shrinks, so the repaint still covers what the
    // shadow used to reach; show only once the new geometry is in place.
    if (!visible)
        setVisible(false);
    // Relative to the window node, whose geometry is the outer window bounds.
    setGeometry(content.marginsAdded(m_margins));
    if (visible)
        setVisible(true);
    // An elevation change alone moves neither, so ask for the repaint here.
    notifyChanged();
}

void ShadowNode::setSurfaceOrigin(const base::Point &origin)
{
    if (origin == m_surfaceOrigin)
        return;
    m_surfaceOrigin = origin;
    notifyChanged();
}

void ShadowNode::paint(render::Renderer &renderer, const base::Rect &bounds) const
{
    if (m_elevation == wm::ShadowElevation::None || m_content.isEmpty())
        return;
    const ShadowGeometry geometry(m_content.size(), m_radius,
                                  m_theme->metrics().shadowFor(m_elevation),
                                  int(std::ceil(renderer.rasterScale())), m_corners, m_silhouette);
    // paintShadow's target rects are already offset by the margins, so it wants
    // the window's own top-left, not the node's.
    const base::PointF surface(bounds.origin() + m_surfaceOrigin);
    paintShadow(renderer, geometry,
                bounds.origin() + base::Point(m_margins.left(), m_margins.top()), 1,
                renderer.pixelAligned(surface) - surface);
}

////////////////////////////////////////////////////////////////////////////////
// Shadow:

Shadow::Shadow(wm::Window *window, Theme *theme)
    : m_window(window)
    , m_theme(theme)
    , m_contentBounds(window->bounds())
{
    if (scene::Node *windowNode = m_window->node()) {
        m_node = static_cast<ShadowNode *>(
                windowNode->addChild(std::make_unique<ShadowNode>(m_theme)));
    }
    updateNode();
}

Shadow::~Shadow()
{
    if (!m_node)
        return;
    ShadowNode *node = m_node;
    m_node = nullptr;
    if (node->parent())
        node->parent()->removeChild(node);
}

void Shadow::setElevation(wm::ShadowElevation elevation)
{
    m_elevation = elevation;
    updateNode();
}

void Shadow::setContentBounds(const base::Rect &bounds)
{
    // Only the size changes what is drawn; the origin comes from the window
    // node this one hangs off. The node ignores a call that changes nothing,
    // so every controller notification can refresh the silhouette cheaply.
    m_contentBounds = bounds;
    updateNode();
}

base::Rect Shadow::shadowBounds() const
{
    if (!m_node || !m_node->isVisible())
        return base::Rect();
    return m_node->geometry().translated(m_contentBounds.origin());
}

void Shadow::stackAtBottom()
{
    if (m_node && m_node->parent())
        m_node->parent()->lower(m_node);
}

void Shadow::updateNode()
{
    if (!m_node)
        return;
    // exo synchronizes shadow bounds with the frame that carries the matching
    // content (ShellSurfaceBase::OnSurfaceCommit allocates a new
    // LocalSurfaceId when they change). Lingmo gets the same guarantee by
    // sizing the shadow from what the window has actually presented, so it can
    // never hug an edge the client has not reached yet.
    base::Rect content = m_window->presentedBounds();
    m_node->setSurfaceOrigin(m_window->frameInsets().topLeft() + m_window->surfaceOrigin());
    if (m_window->role() == wm::WindowRole::Dock && m_window->rootSurface()) {
        // The dock surface includes transparent space outside its rounded panel,
        // which its glass covers exactly.
        const base::Rect glass = m_window->rootSurface()->backgroundBlurRect();
        if (!glass.isEmpty())
            content = content.intersected(glass);
        // The panel's corners grow with the icon size; past the largest shadow
        // corner the shadow keeps that corner rather than disappearing.
        const int panel =
                base::roundToInt(std::min(content.width(), content.height()) * 28.0 / 86.0);
        m_node->update(content, m_elevation, Corners::all(),
                       std::min(panel, ShadowGeometry::kMaxRadius));
        return;
    }
    const auto visual = wm::WindowVisualController::evaluate(*m_window, m_theme->visualStyle());
    // A client-drawn outline casts the shadow instead of the surface rectangle.
    if (!visual.outlinePath.isEmpty()) {
        const ui::Path &outline = visual.outlinePath;
        content = ui::toBase(outline.boundingRect()).toEnclosingRect();
        m_node->update(content, m_elevation, Corners::all(), 0,
                       outline.translated(-content.x(), -content.y()));
        return;
    }
    // So does a glass body inside a larger surface: the transparent room around
    // it, as a banner keeps for the close button on its corner, casts nothing.
    if (!visual.outlineShape.bounds.isEmpty()) {
        content = visual.outlineShape.bounds.toEnclosingRect();
        m_node->update(content, m_elevation, Corners::all(),
                       base::roundToInt(visual.outlineShape.radius));
        return;
    }
    content = visual.shape.bounds.toEnclosingRect();
    const int radius = m_window->isPopup() && m_window->rootSurface()
            ? m_window->rootSurface()->backgroundBlurRadius()
            : (visual.mode == scene::WindowVisualMode::PreserveClient
                       ? WindowVisualStyle::radiusFor(content.size())
                       : int(visual.shape.radius));
    m_node->update(content, m_elevation, Corners::all(), radius);
}

////////////////////////////////////////////////////////////////////////////////
// ShadowProvider:

ShadowProvider::ShadowProvider(Theme *theme)
    : m_theme(theme)
{
}

std::unique_ptr<wm::WindowShadow> ShadowProvider::createShadow(wm::Window *window)
{
    return std::make_unique<Shadow>(window, m_theme);
}

} // namespace Lingmo::Decoration

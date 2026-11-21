// Copyright 2012 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/compositor_extra/shadow.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// The compositor shadow's presentation half. Independent of the SSD frame: a
// client-decorated window with no Decoration still gets one.
//
//   window node
//     +-- ShadowNode         <- this, always at the bottom
//     +-- WindowVisualNode
//          +-- DecorationNode (SSD only)
//          +-- client surface tree

#ifndef LINGMO_DECORATION_SHADOW_NODE_H_
#define LINGMO_DECORATION_SHADOW_NODE_H_

#include "base/geometry.h"
#include "decoration/raster.h"
#include "scene/node.h"
#include "wm/shadow_types.h"
#include "ui/path.h"
#include "wm/window_shadow.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {
class Window;
}

namespace Lingmo::Decoration {

class Theme;

class ShadowNode : public scene::Node
{
public:
    explicit ShadowNode(const Theme *theme);

    // `content` is what the window actually has on screen, in the window node's
    // coordinates, and `corners` and `radius` its silhouette; the node's own
    // geometry becomes that rectangle grown by the style's exterior margins, so
    // the scene - and with it the animation's offscreen coverage and the
    // scanout planner - sees everything the shadow paints. Repeating a call
    // that changes nothing costs nothing.
    // A non-empty `silhouette`, relative to `content`, replaces the corners and radius.
    void update(const base::Rect &content, wm::ShadowElevation elevation, const Corners &corners,
                int radius, const ui::Path &silhouette = {});

    wm::ShadowElevation elevation() const { return m_elevation; }

    // The silhouette the shadow is cut around: the window's own corner shape,
    // not the frame's.
    Corners corners() const { return m_corners; }

    int radius() const { return m_radius; }

    const ui::Path &silhouette() const { return m_silhouette; }

    base::Insets margins() const { return m_margins; }

    // Where the client surface sits in the window, so the shadow moves with it
    // onto physical pixels.
    void setSurfaceOrigin(const base::Point &origin);

    // A drawn shadow is blended over whatever is behind the window, so the
    // frame it appears in has to be composited.
    Content content() const override
    {
        return m_elevation == wm::ShadowElevation::None ? Content::None : Content::Translucent;
    }

protected:
    void paint(render::Renderer &renderer, const base::Rect &bounds) const override;

private:
    const Theme *m_theme = nullptr;
    base::Rect m_content;
    Corners m_corners;
    int m_radius = 0;
    ui::Path m_silhouette;
    base::Point m_surfaceOrigin;
    // What the controller asked for, and what is actually drawn - which is
    // None as well whenever the window is too small for the style.
    wm::ShadowElevation m_requested = wm::ShadowElevation::None;
    wm::ShadowElevation m_elevation = wm::ShadowElevation::None;
    base::Insets m_margins;
};

// What wm::ShadowController drives. Owns the node, which lives under the
// window's own node so that stacking, hiding and the close animation take the
// shadow with them.
class Shadow : public wm::WindowShadow
{
public:
    Shadow(wm::Window *window, Theme *theme);
    Shadow(const Shadow &) = delete;
    Shadow &operator=(const Shadow &) = delete;
    ~Shadow() override;

    void setElevation(wm::ShadowElevation elevation) override;

    wm::ShadowElevation elevation() const override { return m_elevation; }

    void setContentBounds(const base::Rect &bounds) override;

    base::Rect contentBounds() const override { return m_contentBounds; }

    base::Rect shadowBounds() const override;
    void stackAtBottom() override;

    // Null when the window has no scene at all.
    ShadowNode *node() const { return m_node; }

private:
    void updateNode();

    wm::Window *m_window = nullptr;
    Theme *m_theme = nullptr;
    ShadowNode *m_node = nullptr;
    base::Rect m_contentBounds;
    wm::ShadowElevation m_elevation = wm::ShadowElevation::None;
};

class ShadowProvider : public wm::ShadowProvider
{
public:
    explicit ShadowProvider(Theme *theme);

    std::unique_ptr<wm::WindowShadow> createShadow(wm::Window *window) override;

private:
    Theme *m_theme = nullptr;
};

} // namespace Lingmo::Decoration

#endif // LINGMO_DECORATION_SHADOW_NODE_H_

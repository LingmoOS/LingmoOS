// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_SCENE_NODE_H_
#define LINGMO_SCENE_NODE_H_

#include <functional>
#include <memory>
#include <vector>

#include <string>
#include <deque>

#include "render/renderer.h"
#include "base/region.h"

namespace Lingmo::Scene {

// A positioned, ordered item in the compositor scene. Geometry is relative to
// the parent; first child is furthest back. Children normally paint after the
// parent, with an optional split for subsurfaces stacked below parent content.
class Node
{
public:
    explicit Node(std::string name = {});
    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;
    virtual ~Node();

    const std::string &name() const { return m_name; }

    Node *parent() const { return m_parent; }

    const std::vector<Node *> &children() const { return m_childOrder; }

    Node *addChild(std::unique_ptr<Node> child);
    std::unique_ptr<Node> removeChild(Node *child);

    // Moves `child` to the front (top) or back (bottom) of the sibling order.
    void raise(Node *child);
    void lower(Node *child);

    // Puts the children in `order`, which has to list each of them once. Only
    // the ones that have to move change, as aura restacks one window at a time
    // (Window::StackChildAbove): the longest run already in order stays put.
    void reorderChildren(const std::vector<Node *> &order);

    void setChildrenBelowParent(size_t count);

    size_t childrenBelowParent() const { return m_childrenBelowParent; }

    base::Rect geometry() const { return m_geometry; }

    void setGeometry(const base::Rect &geometry);

    base::Point position() const { return m_geometry.topLeft(); }

    void setPosition(const base::Point &position) { setGeometry(base::Rect(position, m_geometry.size())); }

    // Position in output coordinates.
    base::Point absolutePosition() const;
    base::Rect absoluteGeometry() const;

    virtual base::Rect absolutePaintBounds() const { return absoluteGeometry(); }

    bool isVisible() const { return m_visible; }

    void setVisible(bool visible);

    void setChangeCallback(std::function<void()> callback) { m_changed = std::move(callback); }

    void notifyChanged() const;
    void notifyDamage(const base::Region &damage);

    // Runs the change callbacks without changing anything, for a hidden child
    // that still paints. Nothing runs under a hidden ancestor.
    void requestFrame() const;

    uint64_t damageId() const { return m_damageId; }

    uint64_t revision() const { return m_revision; }

    // Counts every change any node reported, so a damage collection can be
    // told apart from one over the same tree.
    static uint64_t mutations() { return s_mutations; }

    uint64_t contentRevision() const { return m_contentRevision; }

    base::Region damageSince(uint64_t revision) const;

    // True only if this node and every ancestor is visible.
    bool isEffectivelyVisible() const;

    // Paints children on either side of this node, skipping invisible subtrees.
    virtual void render(render::Renderer &renderer) const;

    // What this node's own paint() puts inside geometry(). The scanout planner
    // needs it to know what a hardware plane would have to replace.
    enum class Content {
        None, // paints nothing of its own
        Opaque, // covers geometry() with no transparency
        Translucent, // paints, possibly blended
    };

    virtual Content content() const { return Content::None; }

    // The node draws its whole subtree itself - an offscreen pass, a transform,
    // an animation - so the planner must treat the subtree as one blob rather
    // than descending into it. Nothing inside can go on a hardware plane.
    virtual bool paintsSubtree() const { return false; }

    virtual bool requiresFullRepaint() const { return paintsSubtree(); }

    // A hidden node that still draws, such as an animation playing on after
    // its window is gone. Damage follows it the same as a visible one.
    virtual bool paintsWhenHidden() const { return false; }

    // Whether an output with this viewport and scale is about to draw the
    // subtree again whole, as into a texture kept between frames that no
    // longer holds it, rather than only where it changed.
    virtual bool redrawsSubtree(const base::Rect &, double) const { return false; }

    virtual bool acceptsInput(const base::PointF &) const { return true; }

    // Drawn as usual, but left out of the input hit test, as aura's
    // ScopedWindowEventTargetingBlocker does for a window dragged with a drag.
    bool isEventTargetingBlocked() const { return m_eventTargetingBlocked; }

    void setEventTargetingBlocked(bool blocked) { m_eventTargetingBlocked = blocked; }

    // Paints over the whole of the renderer's current output, opaquely, so
    // nothing drawn before it there can show. Its parent then skips those
    // siblings, as viz drops quads an opaque one occludes.
    virtual bool occludesOutput(const render::Renderer &) const { return false; }

    // Draws its siblings [begin, end), all those under it the parent would
    // draw, itself: a full-output animation keeps them as a texture rather
    // than repaint them all each frame. False leaves them to the parent.
    virtual bool renderSiblingsBelow(render::Renderer &, size_t /*begin*/, size_t /*end*/) const
    {
        return false;
    }

    // A visible node in this subtree occludes the output.
    bool subtreeOccludesOutput(const render::Renderer &renderer) const;

    // Set by the display controller for content it has moved onto a hardware
    // plane, so composition leaves the same pixels out.
    bool isPromotedToOverlay() const { return m_promotedToOverlay; }

    void setPromotedToOverlay(bool promoted)
    {
        if (m_promotedToOverlay != promoted)
            ++s_mutations;
        m_promotedToOverlay = promoted;
    }

    // On screen, but never in an image or a video the compositor captures.
    bool isExcludedFromCapture() const { return m_excludedFromCapture; }

    void setExcludedFromCapture(bool excluded) { m_excludedFromCapture = excluded; }

    // render() for a capture: children excluded from capture are left out.
    void renderForCapture(render::Renderer &renderer) const;

    virtual bool isRetired() const { return false; }

    void collectRetired();
    // Ends the animations in this subtree that only wait to be collected, for
    // when no output is going to draw them: a blanked display, an inactive seat.
    virtual void finishRetiring();
    virtual void releaseRenderResources();

protected:
    virtual void paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const;

    // render() whether or not this node itself is visible, for a subtree drawn
    // somewhere of its own while hidden, as a desk preview draws another desk.
    void renderSubtree(render::Renderer &renderer) const;

private:
    // The topmost child that occludes the output, or 0 when none does.
    size_t firstUnoccludedChild(const render::Renderer &renderer) const;

    static uint64_t s_mutations;
    const uint64_t m_damageId;
    mutable uint64_t m_revision = 0;
    uint64_t m_contentRevision = 0;
    std::deque<std::pair<uint64_t, base::Region>> m_damageHistory;
    std::function<void()> m_changed;
    std::string m_name;
    Node *m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children;
    // Paint order; mirrors m_children but is reordered by raise()/lower().
    std::vector<Node *> m_childOrder;
    base::Rect m_geometry;
    bool m_visible = true;
    bool m_promotedToOverlay = false;
    bool m_eventTargetingBlocked = false;
    bool m_excludedFromCapture = false;
    size_t m_childrenBelowParent = 0;
};

// Solid colour rectangle.
class ColorNode : public Node
{
public:
    explicit ColorNode(const render::Color &color, std::string name = {});

    void setColor(const render::Color &color);

    Content content() const override
    {
        if (m_color.a <= 0.0f)
            return Content::None;
        return m_color.a >= 1.0f ? Content::Opaque : Content::Translucent;
    }

protected:
    void paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const override;

private:
    render::Color m_color;
};

// Textured quad. The texture is not owned.
class TextureNode : public Node
{
public:
    explicit TextureNode(render::Texture *texture, std::string name = {});

    render::Texture *texture() const { return m_texture; }

    void setTexture(render::Texture *texture)
    {
        m_texture = texture;
        notifyChanged();
    }

    float alpha() const { return m_alpha; }

    void setAlpha(float alpha)
    {
        if (m_alpha != alpha) {
            m_alpha = alpha;
            notifyChanged();
        }
    }

    Content content() const override { return m_texture ? Content::Translucent : Content::None; }

protected:
    void paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const override;

private:
    render::Texture *m_texture = nullptr;
    float m_alpha = 1.0f;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_NODE_H_

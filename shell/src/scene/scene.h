// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_SCENE_SCENE_H_
#define LINGMO_SCENE_SCENE_H_

#include <memory>

#include "scene/damage_tracker.h"
#include "scene/node.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

// Everything the compositor draws, rooted at a single node covering the output.
class Scene
{
public:
    Scene();
    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;
    ~Scene();

    Node *root() { return m_root.get(); }

    const Node *root() const { return m_root.get(); }

    // Sets the root geometry; nodes are positioned relative to it.
    void setSize(const base::Size &size);

    base::Size size() const { return m_root->geometry().size(); }

    // Clears to `background` and paints the tree back to front.
    void render(render::Renderer &renderer) const;

    // Whether the output `context` describes has anything to repaint since
    // its last frame.
    bool hasDamage(const render::OutputRenderContext &context) const;

    // Whether a visible node that repaints the whole of `viewport` whenever it
    // draws (an animation covering the output) lies on it.
    bool requiresFullRepaint(const base::Rect &viewport) const;

    // Ends and collects closing windows while no output composites: their
    // snapshots would otherwise pile up until the next frame, as retirement
    // normally happens after painting.
    void retireClosing();

    render::Color background() const { return m_background; }

    void setBackground(const render::Color &color)
    {
        m_background = color;
        m_root->notifyChanged();
    }

private:
    base::Rect viewportFor(const render::OutputRenderContext &context) const;

    mutable std::map<std::string, DamageTracker> m_damageTrackers;
    std::unique_ptr<Node> m_root;
    render::Color m_background;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_SCENE_H_

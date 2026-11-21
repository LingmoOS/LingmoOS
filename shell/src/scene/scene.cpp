// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/scene.h"

#include <optional>

#include "scene/overlay_candidate.h"
#include "base/region.h"

namespace Lingmo::Scene {

Scene::Scene()
    : m_root(std::make_unique<Node>("root"))
{
}

Scene::~Scene() = default;

void Scene::setSize(const base::Size &size)
{
    m_root->setGeometry(base::Rect(base::Point(0, 0), size));
}

base::Rect Scene::viewportFor(const render::OutputRenderContext &context) const
{
    return context.logicalViewport.isEmpty() ? m_root->geometry() : context.logicalViewport;
}

bool Scene::requiresFullRepaint(const base::Rect &viewport) const
{
    const std::function<bool(const Node &)> visit = [&](const Node &node) {
        if (!node.isVisible())
            return false;
        if (node.requiresFullRepaint() && node.absoluteGeometry().intersects(viewport))
            return true;
        for (const Node *child : node.children()) {
            if (visit(*child))
                return true;
        }
        return false;
    };
    return visit(*m_root);
}

bool Scene::hasDamage(const render::OutputRenderContext &context) const
{
    const auto tracker = m_damageTrackers.find(context.outputId);
    return tracker == m_damageTrackers.end()
            || tracker->second.hasDamage(*m_root, viewportFor(context), context.scale,
                                         context.transform);
}

void Scene::retireClosing()
{
    m_root->finishRetiring();
    m_root->collectRetired();
}

void Scene::render(render::Renderer &renderer) const
{
    std::optional<render::BackdropPass> backdrops;
    if (!renderer.capturing) {
        const auto &context = renderer.outputContext();
        auto &tracker = m_damageTrackers[context.outputId];
        const base::Region damage =
                tracker.update(*m_root, viewportFor(context), context.scale, context.transform);
        // Each output's tracker stands for what its target holds.
        const auto scope = uint64_t(reinterpret_cast<uintptr_t>(&tracker));
        const auto kept = [&](uint64_t owner) {
            return renderer.keepsBackdrop(scope, owner);
        };
        renderer.prepareDamage(damage, tracker.samples(kept));
        backdrops.emplace(renderer, scope, tracker.freshBackdrops(kept));
    }
    // viz leaves an opaque root pass uncleared (SkiaRenderer::ClearFramebuffer);
    // under a node covering the output, a clear is a full-screen write for nothing.
    if (!m_root->subtreeOccludesOutput(renderer))
        renderer.clear(m_background);
    m_root->render(renderer);
    if (renderer.presentationSubmission)
        removeOccludedPresentation(*m_root, m_root->geometry().size(),
                                   *renderer.presentationSubmission);
    if (!renderer.capturing)
        m_root->collectRetired();
}

} // namespace Lingmo::Scene

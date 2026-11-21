// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/display.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "compositor/compositor.h"

#include <algorithm>
#include <limits>

#include "compositor/dmabuf_buffer.h"
#include "compositor/shm_pool.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "scene/node.h"
#include "scene/surface_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Compositor {

namespace {

// The centre of the last pixels inside: an even extent rounds toward the origin.
base::Point innerCenter(const base::Rect &rect)
{
    return {int((int64_t(rect.x()) * 2 + rect.width() - 1) / 2),
            int((int64_t(rect.y()) * 2 + rect.height() - 1) / 2)};
}

// Containment that is never true when either has neither width nor height.
bool containsNonNull(const base::Rect &outer, const base::Rect &inner)
{
    const auto null = [](const base::Rect &rect) {
        return rect.width() == 0 && rect.height() == 0;
    };
    return !null(outer) && !null(inner) && outer.contains(inner);
}

} // namespace

namespace {
constexpr int kCascadeStep = 40;
constexpr int kCascadeCount = 8;
} // namespace

Compositor::Compositor(scene::Node *surfaceContainer)
    : m_surfaceContainer(surfaceContainer)
{
}

Compositor::~Compositor() = default;

std::unique_ptr<Surface> Compositor::createSurface()
{
    return std::make_unique<Surface>(this);
}

std::shared_ptr<ShmPool> Compositor::createShmPool(int fd, int32_t size)
{
    return ShmPool::create(fd, size);
}

std::shared_ptr<Buffer> Compositor::createDmaBufBuffer(render::DmaBufAttributes attributes)
{
    return DmaBufBuffer::create(m_dmaBufDevice, std::move(attributes));
}

scene::SurfaceNode *Compositor::createSurfaceNode(Surface *surface)
{
    m_surfaces.push_back(surface);
    LM_DEBUG(lingmoAurora) << "Surface created; total" << m_surfaces.size();

    if (!m_surfaceContainer)
        return nullptr;

    const int step = (m_placementIndex++ % kCascadeCount) * kCascadeStep;
    auto *node = static_cast<scene::SurfaceNode *>(m_surfaceContainer->addChild(
            std::make_unique<scene::SurfaceNode>("surface")));
    node->setPosition(base::Point(step, step));
    surface->setPosition(base::Point(step, step));
    m_surfaceByNode.emplace(node, surface);
    return node;
}

Surface *Compositor::surfaceForNode(const scene::Node *node) const
{
    const auto found = m_surfaceByNode.find(node);
    return found == m_surfaceByNode.end() ? nullptr : found->second;
}

void Compositor::surfaceDestroyed(Surface *surface)
{
    m_surfaceByNode.erase(surface->node());
    m_surfaces.erase(std::remove(m_surfaces.begin(), m_surfaces.end(), surface), m_surfaces.end());
    LM_DEBUG(lingmoAurora) << "Surface destroyed; total" << m_surfaces.size();
    scheduleRepaint();
}

void Compositor::updateTextures(render::Renderer &renderer)
{
    if (m_contentProducer)
        m_contentProducer->produceContent(renderer);
    for (Surface *surface : m_surfaces)
        surface->updateTexture(renderer);
}

void Compositor::frameSubmitted(uint64_t generation)
{
    for (Surface *surface : m_surfaces)
        surface->frameSubmitted(generation);
}

void Compositor::frameSubmittedForOutput(const base::Rect &viewport, uint64_t generation)
{
    for (Surface *surface : m_surfaces) {
        const auto *node = surface->node();
        if (node && node->absoluteGeometry().intersects(viewport))
            surface->frameSubmitted(generation);
    }
}

void Compositor::abandonUnsubmittedPresentation(const base::Rect &viewport,
                                                const render::PresentationSubmission &submission)
{
    // Discarding runs client callbacks, which may destroy any surface.
    std::vector<std::shared_ptr<Surface *>> snapshot;
    for (Surface *surface : m_surfaces)
        snapshot.push_back(surface->lifetimeToken());
    for (const auto &token : snapshot) {
        Surface *surface = *token;
        const scene::Node *node = surface ? surface->node() : nullptr;
        if (!node)
            continue;
        const base::Rect bounds = node->absoluteGeometry();
        const bool elsewhere = std::any_of(
                m_outputViews.begin(), m_outputViews.end(), [&](const OutputView &output) {
                    return output.bounds.intersects(bounds);
                });
        if (node->isEffectivelyVisible() && !containsNonNull(viewport, bounds) && elsewhere)
            continue;
        surface->abandonUnsubmittedPresentation(submission);
    }
}

void Compositor::frameDiscarded(uint64_t generation)
{
    outputDiscarded(generation);
    for (Surface *surface : m_surfaces)
        surface->frameDiscarded(generation);
}

void Compositor::frameDrawn(uint32_t timeMs, uint64_t generation)
{
    // Completion may destroy any surface or construct a replacement at its address.
    std::vector<std::shared_ptr<Surface *>> snapshot;
    for (Surface *surface : m_surfaces)
        snapshot.push_back(surface->lifetimeToken());
    for (const auto &token : snapshot)
        if (*token)
            (*token)->frameDrawn(timeMs, generation);
}

void Compositor::setOutputScale(OutputScale scale)
{
    if (scale == m_outputScale)
        return;
    m_outputScale = scale;
    outputScaleChanged();
    refreshOutputScales();
    scheduleRepaint();
}

void Compositor::setOutputViews(std::vector<OutputView> outputs)
{
    if (m_outputViews == outputs)
        return;
    m_outputViews = std::move(outputs);
    outputViewsChanged();
    refreshOutputScales();
    scheduleRepaint();
}

void Compositor::refreshOutputScales()
{
    std::vector<std::shared_ptr<Surface *>> snapshot;
    for (auto *surface : m_surfaces)
        snapshot.push_back(surface->lifetimeToken());
    for (const auto &token : snapshot) {
        if (*token)
            (*token)->notifyOutputScaleChanged();
    }
}

std::vector<std::shared_ptr<Surface *>> Compositor::takeMovedSurfaces()
{
    std::vector<std::shared_ptr<Surface *>> moved;
    for (Surface *surface : m_surfaces) {
        if (surface->updatePlacement())
            moved.push_back(surface->lifetimeToken());
    }
    return moved;
}

bool Compositor::awaitsFrame(const base::Rect &viewport) const
{
    if (m_contentProducer && m_contentProducer->hasPendingContent(viewport))
        return true;
    return std::any_of(m_surfaces.begin(), m_surfaces.end(), [&](const Surface *surface) {
        const auto *node = surface->node();
        return node && node->absoluteGeometry().intersects(viewport) && surface->awaitsFrame();
    });
}

uint32_t Compositor::preferredScaleFor(const Surface *surface) const
{
    if (!surface || m_outputViews.empty() || !surface->node())
        return m_outputScale.numerator();
    uint32_t result = 0;
    const base::Rect geometry = surface->node()->absoluteGeometry();
    for (const OutputView &output : m_outputViews) {
        if (geometry.intersects(output.bounds))
            result = std::max(result, output.scale.numerator());
    }
    if (result)
        return result;
    // Unmapped and displaced surfaces still need a current output density.
    const OutputView *nearest = nullptr;
    int64_t distance = std::numeric_limits<int64_t>::max();
    const base::Point position = innerCenter(geometry);
    for (const OutputView &output : m_outputViews) {
        if (output.bounds.isEmpty())
            continue;
        const int64_t dx = int64_t(position.x())
                - std::clamp(position.x(), output.bounds.left(), output.bounds.right() - 1);
        const int64_t dy = int64_t(position.y())
                - std::clamp(position.y(), output.bounds.top(), output.bounds.bottom() - 1);
        const int64_t candidate = std::abs(dx) + std::abs(dy);
        if (candidate < distance) {
            distance = candidate;
            nearest = &output;
        }
    }
    return nearest ? nearest->scale.numerator() : m_outputScale.numerator();
}

void Compositor::scheduleRepaint(bool decoration)
{
    repaintNeeded(decoration);
}

} // namespace Lingmo::Compositor

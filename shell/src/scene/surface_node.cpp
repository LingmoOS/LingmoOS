// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/surface_node.h"
#include "base/region.h"

namespace Lingmo::Scene {

SurfaceNode::SurfaceNode(std::string name)
    : Node(std::move(name))
{
}

SurfaceNode::~SurfaceNode() = default;

void SurfaceNode::setTexture(std::shared_ptr<render::Texture> texture)
{
    const bool mappingChanged = bool(m_texture) != bool(texture);
    m_texture = std::move(texture);
    if (m_texture)
        m_texture->contentOpaque = m_opaque;
    if (mappingChanged)
        notifyChanged();
}

std::shared_ptr<render::Texture> SurfaceNode::takeTexture()
{
    notifyChanged();
    return std::move(m_texture);
}

void SurfaceNode::setScanoutBuffer(std::shared_ptr<render::ScanoutBuffer> buffer)
{
    if (m_scanoutBuffer == buffer)
        return;
    m_scanoutBuffer = std::move(buffer);
}

void SurfaceNode::setOpaque(bool opaque)
{
    if (m_opaque == opaque)
        return;
    m_opaque = opaque;
    // An alpha buffer under a full opaque region is drawn opaque, as content()
    // promises; blended, a backdrop darkened in place keeps the previous frame.
    if (m_texture)
        m_texture->contentOpaque = opaque;
    notifyChanged();
}

void SurfaceNode::setBackgroundBlur(const base::Rect &rect, int radius, const render::GlassParams &glass,
                                    const ui::Path &shape)
{
    if (m_blurRect == rect && m_blurRadius == radius && m_glass == glass && m_blurShape == shape)
        return;
    m_blurRect = rect;
    m_blurRadius = radius;
    m_blurShape = shape;
    m_glass = glass;
    notifyChanged();
}

void SurfaceNode::render(render::Renderer &renderer) const
{
    if (!isVisible() && m_hiddenWithWindow && renderer.revealHiddenSurfaces) {
        renderSubtree(renderer);
        return;
    }
    Node::render(renderer);
}

void SurfaceNode::paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const
{
    // Content the display controller put on a hardware plane must not also be
    // composited, or the two would blend on top of each other.
    if (!m_texture || absoluteGeometry.isEmpty() || (isPromotedToOverlay() && !renderer.capturing))
        return;
    // Left out of a partial repaint, the surface is still the one on screen.
    if (!renderer.drawsInto(absoluteGeometry)) {
        if (m_alpha > 0)
            renderer.recordPresentation(m_presentationContent, absoluteGeometry);
        return;
    }
    // The client may still be writing this buffer; the wait is queued on the
    // GPU, so composition does not block the compositor thread.
    if (hasBackgroundBlur() && !renderer.suppressBackgroundBlur)
        renderer.drawBackgroundBlur(m_blurRect.translated(absoluteGeometry.topLeft()), m_blurRadius,
                                    absoluteGeometry, m_glass,
                                    m_blurShape.translated(absoluteGeometry.x(), absoluteGeometry.y()), damageId());
    renderer.waitFence(m_acquireFence);
    const float alpha = m_alpha * renderer.opacityScale;
    if (m_contentShape) {
        const auto region = *m_contentShape & base::Region(base::Rect(base::Point(), absoluteGeometry.size()));
        for (const auto &rect : region) {
            renderer.pushRoundedClip({base::RectF(rect.translated(absoluteGeometry.topLeft())), 0});
            renderer.drawTextureMapped(m_texture.get(), absoluteGeometry, alpha, m_textureMapping);
            renderer.popRoundedClip();
        }
    } else {
        renderer.drawTextureMapped(m_texture.get(), absoluteGeometry, alpha, m_textureMapping);
    }
    if (m_alpha > 0)
        renderer.recordPresentation(m_presentationContent, absoluteGeometry);
}

} // namespace Lingmo::Scene

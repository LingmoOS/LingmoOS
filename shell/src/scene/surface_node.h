// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_SCENE_SURFACE_NODE_H_
#define LINGMO_SCENE_SURFACE_NODE_H_

#include <memory>
#include <optional>


#include "render/glass.h"
#include "render/presentation.h"
#include "render/scanout.h"
#include "render/sync_fence.h"
#include "scene/node.h"
#include "base/region.h"
#include "base/transform.h"
#include "ui/path.h"

namespace Lingmo::Scene {

// The renderable content of one compositor::Surface. Unlike TextureNode it owns
// its texture, which the compositor produces from the surface's committed
// buffer.
class SurfaceNode : public Node
{
public:
    explicit SurfaceNode(std::string name = {});
    ~SurfaceNode() override;

    const base::AffineTransform &textureMapping() const { return m_textureMapping; }

    void setTextureMapping(const base::AffineTransform &mapping)
    {
        if (m_textureMapping == mapping)
            return;
        m_textureMapping = mapping;
        notifyChanged();
    }

    const std::shared_ptr<render::PresentationContent> &presentationContent() const
    {
        return m_presentationContent;
    }

    void setPresentationContent(std::shared_ptr<render::PresentationContent> content)
    {
        m_presentationContent = std::move(content);
    }

    render::Texture *texture() const { return m_texture.get(); }

    // Shared rather than unique because a DMA-BUF texture is imported once and
    // owned by the wl_buffer, which may outlive any one commit.
    void setTexture(std::shared_ptr<render::Texture> texture);
    std::shared_ptr<render::Texture> takeTexture();

    // `shape` is surface-local; empty for the rounded rectangle.
    void setBackgroundBlur(const base::Rect &rect, int radius, const render::GlassParams &glass = {},
                           const ui::Path &shape = {});

    bool hasBackgroundBlur() const { return !m_blurRect.isEmpty(); }
    base::Rect backgroundBlurRect() const { return m_blurRect; }
    int backgroundBlurRadius() const { return m_blurRadius; }
    const render::GlassParams &glass() const { return m_glass; }
    const ui::Path &backgroundBlurShape() const { return m_blurShape; }

    // Mapped, but hidden with its window: drawn anyway where the renderer
    // reveals hidden surfaces.
    void setHiddenWithWindow(bool hidden) { m_hiddenWithWindow = hidden; }
    bool isHiddenWithWindow() const { return m_hiddenWithWindow; }

    float alpha() const { return m_alpha; }

    void setAlpha(float alpha)
    {
        if (m_alpha != alpha) {
            m_alpha = alpha;
            notifyChanged();
        }
    }

    // The client storage behind the texture, when the display controller may be
    // able to scan it out instead of compositing it. Null for shm content and
    // for anything the compositor drew itself.
    const std::shared_ptr<render::ScanoutBuffer> &scanoutBuffer() const { return m_scanoutBuffer; }

    void setScanoutBuffer(std::shared_ptr<render::ScanoutBuffer> buffer);

    // Whether the content is known to be fully opaque over its whole geometry,
    // which is what lets it take the primary plane or an opaque overlay.
    bool isOpaque() const { return m_opaque && !m_contentShape; }

    void setContentShape(std::optional<base::Region> shape)
    {
        if (m_contentShape == shape)
            return;
        m_contentShape = std::move(shape);
        notifyChanged();
    }

    bool hasContentShape() const { return m_contentShape.has_value(); }

    void setOpaque(bool opaque);

    // Signalled when the client has finished writing the current buffer. Every
    // reader - the GPU for composition, KMS for scanout - waits on it first.
    const render::SyncFence &acquireFence() const { return m_acquireFence; }

    void setAcquireFence(render::SyncFence fence) { m_acquireFence = std::move(fence); }

    Content content() const override
    {
        if (!m_texture)
            return Content::None;
        return !hasBackgroundBlur() && isOpaque() && m_alpha >= 1.0f ? Content::Opaque
                                                                     : Content::Translucent;
    }

    void render(render::Renderer &renderer) const override;

protected:
    void paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const override;

private:
    std::shared_ptr<render::PresentationContent> m_presentationContent;
    base::AffineTransform m_textureMapping;
    std::shared_ptr<render::Texture> m_texture;
    std::shared_ptr<render::ScanoutBuffer> m_scanoutBuffer;
    render::SyncFence m_acquireFence;
    base::Rect m_blurRect;
    int m_blurRadius = 0;
    bool m_hiddenWithWindow = false;
    ui::Path m_blurShape;
    render::GlassParams m_glass;
    float m_alpha = 1.0f;
    bool m_opaque = false;
    std::optional<base::Region> m_contentShape;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_SURFACE_NODE_H_

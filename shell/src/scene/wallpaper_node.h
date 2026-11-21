// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_SCENE_WALLPAPER_NODE_H_
#define LINGMO_SCENE_WALLPAPER_NODE_H_

#include <memory>
#include <optional>

#include "scene/node.h"
#include "ui/bitmap.h"

namespace Lingmo::Scene {

// One output's desktop background, at the bottom of the desktop layer: the
// wallpaper decoded at the output's device size, or a plain colour, and on
// the primary output the gradient that lifts the status bar's contents.
// Opaque, so the overview darkens it in place and a desk switch slides it
// with the desk's windows.
class WallpaperNode final : public Node
{
public:
    WallpaperNode();
    ~WallpaperNode() override;

    // The picture, premultiplied 0xAARRGGBB covering the node; null shows the
    // colour instead.
    void setPicture(const ui::Bitmap &picture);
    const ui::Bitmap &picture() const { return m_picture; }
    void setColor(const render::Color &color);
    render::Color color() const { return m_color; }

    // The gradient under the status bar, `height` logical pixels from the
    // top, `color` fading out along BarBackdrop's stops; none when unset.
    struct Gradient {
        ui::Color color;
        int height = 0;

        bool operator==(const Gradient &) const = default;
    };
    void setGradient(std::optional<Gradient> gradient);
    const std::optional<Gradient> &gradient() const { return m_gradient; }

    // Drawn into `target` rather than its own geometry, as the overview's
    // desk previews draw it.
    void renderMapped(render::Renderer &renderer, const base::RectF &target) const;

    Content content() const override { return Content::Opaque; }
    bool acceptsInput(const base::PointF &) const override { return false; }
    void releaseRenderResources() override;

protected:
    void paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const override;

private:
    render::Texture *pictureTexture(render::Renderer &renderer) const;
    render::Texture *gradientTexture(render::Renderer &renderer) const;
    void paintInto(render::Renderer &renderer, const base::RectF &target, bool mapped) const;

    ui::Bitmap m_picture;
    render::Color m_color {0, 0, 0, 1};
    std::optional<Gradient> m_gradient;

    mutable std::unique_ptr<render::Texture> m_pictureTexture;
    mutable const render::Renderer *m_pictureRenderer = nullptr;
    mutable bool m_pictureStale = true;
    mutable std::unique_ptr<render::Texture> m_gradientTexture;
    mutable const render::Renderer *m_gradientRenderer = nullptr;
    mutable std::optional<Gradient> m_gradientDrawn;
    mutable int m_gradientRows = 0;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_WALLPAPER_NODE_H_

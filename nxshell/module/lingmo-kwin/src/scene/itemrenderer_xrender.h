#pragma once

#include "scene/itemrenderer.h"
#include "scene/surfaceitem_x11.h"
#include "kwineffects.h"
#include "kwinglutils.h"

#include <xcb/render.h>
#include <xcb/xfixes.h>

#include <memory>

class QPainter;

namespace KWin
{

class DecorationItem;
class SurfaceItem;
class ShadowItem;
class Window;

enum ImageFilterType {
    ImageFilterFast,
    ImageFilterGood,
};

class KWIN_EXPORT ItemRendererXRender : public ItemRenderer
{
public:
    ItemRendererXRender();
    ~ItemRendererXRender() override;

    void beginFrame(RenderTarget *renderTarget) override;
    void endFrame() override;

    void renderBackground(const QRegion &region) override;
    void renderItem(Item *item, int mask, const QRegion &region, const WindowPaintData &data) override;

    ImageItem *createImageItem(Scene *scene, Item *parent = nullptr) override;

private:
    void renderSurfaceItem(SurfaceItem *surfaceItem, int mask, const QRegion &region, const WindowPaintData &data) const;
    void renderDecorationItem(DecorationItem *decorationItem, int mask, const QRegion &region, const WindowPaintData &data) const;
    void renderImageItem(ImageItem *imageItem) const;
    void renderShadowItem(ShadowItem *shadowItem) const;
    void renderItem(Item *item) const;

    void setPictureFilter(xcb_render_picture_t, ImageFilterType) const;
    xcb_render_picture_t xrenderBufferPicture() const;

    xcb_render_picture_t m_targetBuffer;

    static XRenderPicture *s_fadeAlphaPicture;
};

} // namespace KWin

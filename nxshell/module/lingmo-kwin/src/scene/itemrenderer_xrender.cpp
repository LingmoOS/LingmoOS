#include "scene/itemrenderer_xrender.h"

#include "utils/xcbutils.h"
#include "../backends/x11/common/kwinxrenderutils.h"
#include "platformsupport/scenes/xrender/xrendersurfacetexture.h"
#include "workspacescene_xrender.h"
#include "scene/imageitem.h"
#include "scene/windowitem.h"
#include "scene/shadowitem.h"
#include "scene/decorationitem.h"
#include "scene/workspacescene_xrender.h"
#include <kwineffects.h>
#include "../platformsupport/scenes/xrender/xrendersurfacetexture_x11.h"


namespace KWin
{

XRenderPicture * ItemRendererXRender::s_fadeAlphaPicture = nullptr;

ItemRendererXRender::ItemRendererXRender()
{
}

ItemRendererXRender::~ItemRendererXRender()
{
}

ImageItem *ItemRendererXRender::createImageItem(Scene *scene, Item *parent)
{
    return new ImageItem(scene, parent);
}


xcb_render_picture_t ItemRendererXRender::xrenderBufferPicture() const
{
    return m_targetBuffer;
}

void ItemRendererXRender::beginFrame(RenderTarget *renderTarget)
{
    xcb_render_picture_t * targetBuffer = std::get<xcb_render_picture_t *>(renderTarget->nativeHandle());
    m_targetBuffer = *targetBuffer;
}

void ItemRendererXRender::endFrame()
{
}

void ItemRendererXRender::renderBackground(const QRegion &region)
{
    if (region.isEmpty()) {
        return;
    }
    xcb_render_color_t col = { 0, 0, 0, 0xffff }; // black
    const QVector<xcb_rectangle_t> &rects = Xcb::regionToRects(region);
    xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, xrenderBufferPicture(), col, rects.count(), rects.data());
}

void ItemRendererXRender::renderItem(Item *item, int mask, const QRegion &_region, const WindowPaintData &data)
{
    QRegion region = _region;

    const QRect boundingRect = item->mapToGlobal(item->boundingRect()).toAlignedRect();
    if (!(mask & (Effect::PAINT_WINDOW_TRANSFORMED | Effect::PAINT_SCREEN_TRANSFORMED))) {
        region &= boundingRect;
    }

    if (region.isEmpty()) {
        return;
    }

    const QList<Item *> sortedChildItems = item->sortedChildItems();

    for (Item *childItem : sortedChildItems) {
        if (childItem->z() >= 0) {
            break;
        }
        if (childItem->explicitVisible()) {
            renderItem(childItem, mask, _region, data);
        }
    }

    item->preprocess();
    if (auto surfaceItem = qobject_cast<SurfaceItem *>(item)) {
        renderSurfaceItem(surfaceItem, mask, _region, data);
    } else if (auto decorationItem = qobject_cast<DecorationItem *>(item)) {
        renderDecorationItem(decorationItem, mask, _region, data);
    } else if (auto imageItem = qobject_cast<ImageItem *>(item)) {
        renderImageItem(imageItem);
    } else if (auto shadowItem = qobject_cast<ShadowItem *>(item)) {
        renderShadowItem(shadowItem);
    }

    for (Item *childItem : sortedChildItems) {
        if (childItem->z() < 0) {
            continue;
        }
        if (childItem->explicitVisible()) {
            renderItem(childItem, mask, _region, data);
        }
    }

}

void ItemRendererXRender::setPictureFilter(xcb_render_picture_t pic, ImageFilterType filter) const
{
    QByteArray filterName;
    switch (filter) {
    case ImageFilterFast:
        filterName = QByteArray("fast");
        break;
    case ImageFilterGood:
        filterName = QByteArray("good");
        break;
    }
    xcb_render_set_picture_filter(connection(), pic, filterName.length(), filterName.constData(), 0, nullptr);
}

void ItemRendererXRender::renderSurfaceItem(SurfaceItem *surfaceItem, int mask, const QRegion &region, const WindowPaintData &data) const
{
    auto surfaceItemX11 = static_cast<SurfaceItemX11 *>(surfaceItem);
    if (!surfaceItemX11 || !surfaceItemX11->pixmap() || !surfaceItemX11->pixmap()->isValid()) {
        qCWarning(KWIN_CORE, "SurfaceItem is error!");
        return;
    }

    SurfacePixmap *surfacePixmap = surfaceItem->pixmap();
    auto surfacePixmapX11 = dynamic_cast<SurfacePixmapX11 *>(surfacePixmap);

    if (!surfacePixmapX11 || !surfacePixmapX11->isValid()) {
        qCWarning(KWIN_CORE, "surfacePixmapX11 is error!");
        return;
    }

    Window * win = surfaceItemX11->window();
    WindowItem * winItem = win->windowItem();
    const DecorationItem *decItem = winItem->decorationItem();
    const ShadowItem *shadowItem = winItem->shadowItem();

    WorkspaceSceneXRender * xrenderScene = static_cast<WorkspaceSceneXRender *>(surfaceItem->scene());
    if ( win->window() == xrenderScene->backend()->overlayWindow()->window() ) {
        qWarning()<<__FILE__<< __FUNCTION__<<__LINE__<< "SurfaceItem is overlaywindow";
        return;
    }

    bool opaque = !surfaceItemX11->opaque().isEmpty();

    XRenderSurfaceTextureX11 *platformSurfaceTexture =
            static_cast<XRenderSurfaceTextureX11 *>(surfacePixmap->texture());

    if (!platformSurfaceTexture)  {
        qCWarning(KWIN_CORE, "platformSurfaceTexture is null!");
        return;
    }

    if (platformSurfaceTexture && !platformSurfaceTexture->isValid()) {
        if (!platformSurfaceTexture->create()) {
            qCWarning(KWIN_CORE, "Failed to create platform surface texture for window 0x%x",
                      win->frameId());
            return;
        }
    }

    const xcb_render_picture_t pic = platformSurfaceTexture->picture();
    if (pic == XCB_RENDER_PICTURE_NONE)   // The render format can be null for GL and/or Xv visuals
        return;
    surfaceItemX11->resetDamage();

    // set picture filter
    ImageFilterType filter;
    if (options->isXrenderSmoothScale()) { // only when forced, it's slow
        if (mask & Effect::PAINT_WINDOW_TRANSFORMED)
            filter = ImageFilterGood;
        else if (mask & Effect::PAINT_SCREEN_TRANSFORMED)
            filter = ImageFilterGood;
        else
            filter = ImageFilterFast;
    } else
        filter = ImageFilterFast;

    // // do required transformations

    QRectF cr = QRectF(win->clientPos(), win->clientSize());
    QRectF dr = surfaceItem->mapToGlobal(cr).toAlignedRect();

    // clipping image by shape bounding
    QRegion transformed_shape;
    if (win && !win->noBorder()) {
        // decorated client
        transformed_shape = QRegion(0, 0, win->width(), win->height());
        if (win->shape()) {
            // "xeyes" + decoration
            transformed_shape -= cr.toRect();
            for (QRectF& rect : surfaceItem->shape()) {
                transformed_shape = transformed_shape.united(rect.toRect());
            }
        }
    } else {
        for (QRectF& rect : surfaceItem->shape()) {
            transformed_shape = transformed_shape.united(rect.toRect());
        }
    }
    xcb_xfixes_set_picture_clip_region(connection(), pic, XFixesRegion(transformed_shape), 0, 0);

    xcb_render_transform_t xform = {
        DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0),
        DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0),
        DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1)
    };
    static const xcb_render_transform_t identity = {
        DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0),
        DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0),
        DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1)
    };

    xcb_render_picture_t renderTarget = xrenderBufferPicture();

    xcb_render_set_picture_transform(connection(), pic, xform);
    setPictureFilter(pic, filter);

    //BEGIN OF STUPID RADEON HACK
    // This is needed to avoid hitting a fallback in the radeon driver.
    // The Render specification states that sampling pixels outside the
    // source picture results in alpha=0 pixels. This can be achieved by
    // setting the border color to transparent black, but since the border
    // color has the same format as the texture, it only works when the
    // texture has an alpha channel. So the driver falls back to software
    // when the repeat mode is RepeatNone, the picture has a non-identity
    // transformation matrix, and doesn't have an alpha channel.
    // Since we only scale the picture, we can work around this by setting
    // the repeat mode to RepeatPad.
    if (!win->hasAlpha()) {
        const uint32_t values[] = {XCB_RENDER_REPEAT_PAD};
        xcb_render_change_picture(connection(), pic, XCB_RENDER_CP_REPEAT, values);
    }

    const int clientRenderOp = opaque ? XCB_RENDER_PICT_OP_SRC : XCB_RENDER_PICT_OP_OVER;

    if (!(win && win->isShade())) {
        xcb_render_picture_t clientAlpha = XCB_RENDER_PICTURE_NONE;
        if (!opaque) {
            clientAlpha = XRenderUtils::xRenderBlendPicture(data.opacity());
        }
        XFixesRegion xregion(region);
        xcb_xfixes_set_picture_clip_region(connection(), renderTarget, xregion, 0, 0);

        xcb_render_composite(connection(), clientRenderOp, pic, clientAlpha, renderTarget,
                                cr.x(), cr.y(), 0, 0, dr.x(), dr.y(), dr.width(), dr.height());
    }
}

void ItemRendererXRender::renderDecorationItem(DecorationItem *decorationItem, int mask, const QRegion &region, const WindowPaintData &data) const
{
    // //BEGIN deco preparations
    bool noBorder = true;
    xcb_render_picture_t left   = XCB_RENDER_PICTURE_NONE;
    xcb_render_picture_t top    = XCB_RENDER_PICTURE_NONE;
    xcb_render_picture_t right  = XCB_RENDER_PICTURE_NONE;
    xcb_render_picture_t bottom = XCB_RENDER_PICTURE_NONE;
    QRectF dtr, dlr, drr, dbr;
    const SceneXRenderDecorationRenderer *renderer = nullptr;
    Window * win = decorationItem->window();
    if (decorationItem) {
        renderer = static_cast<const SceneXRenderDecorationRenderer *>(decorationItem->renderer());
        noBorder = false;
        if (win) {
            win->layoutDecorationRects(dlr, dtr, drr, dbr);
        }
    }
    if (renderer) {
        left   = renderer->picture(SceneXRenderDecorationRenderer::DecorationPart::Left);
        top    = renderer->picture(SceneXRenderDecorationRenderer::DecorationPart::Top);
        right  = renderer->picture(SceneXRenderDecorationRenderer::DecorationPart::Right);
        bottom = renderer->picture(SceneXRenderDecorationRenderer::DecorationPart::Bottom);
    }
    if (!noBorder) {
        dtr = decorationItem->mapToGlobal(dtr).toAlignedRect();
        dlr = decorationItem->mapToGlobal(dlr).toAlignedRect();
        drr = decorationItem->mapToGlobal(drr).toAlignedRect();
        dbr = decorationItem->mapToGlobal(dbr).toAlignedRect();
    }

    //END deco preparations
    xcb_render_picture_t renderTarget = xrenderBufferPicture();

    XFixesRegion xregion(region);
        xcb_xfixes_set_picture_clip_region(connection(), renderTarget, xregion, 0, 0);

    if (win) {
        if (!noBorder) {
            xcb_render_picture_t decorationAlpha = XRenderUtils::xRenderBlendPicture(data.opacity());
            auto renderDeco = [decorationAlpha, renderTarget](xcb_render_picture_t deco, const QRectF &rect) {
                if (deco == XCB_RENDER_PICTURE_NONE) {
                    return;
                }
                xcb_render_composite(connection(), XCB_RENDER_PICT_OP_OVER, deco, decorationAlpha, renderTarget,
                                        0, 0, 0, 0, rect.x(), rect.y(), rect.width(), rect.height());
            };
            renderDeco(top, dtr);
            renderDeco(left, dlr);
            renderDeco(right, drr);
            renderDeco(bottom, dbr);
        }
    }
}

void ItemRendererXRender::renderShadowItem(ShadowItem *shadowItem) const
{
}

void ItemRendererXRender::renderImageItem(ImageItem *imageItem) const
{
}

void ItemRendererXRender::renderItem(Item *item) const
{
}

}
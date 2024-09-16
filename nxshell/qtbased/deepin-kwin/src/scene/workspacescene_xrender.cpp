/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2006 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2009 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "workspacescene_xrender.h"
#include "../platformsupport/scenes/xrender/xrendersurfacetexture_x11.h"
#include "utils/common.h"
#include "xrenderbackend.h"

#include "window.h"
#include "x11window.h"
#include "composite.h"
#include "deleted.h"
#include "effects.h"
#include "main.h"
#include "core/overlaywindow.h"
#include "core/outputbackend.h"
#include "core/renderloop.h"
#include "core/output.h"
#include "shadowitem.h"
#include "surfaceitem_x11.h"
#include "windowitem.h"
#include "utils/xcbutils.h"
#include "decorations/decoratedclient.h"
#include "itemrenderer_xrender.h"

#include <kwineffects.h>
#include "../backends/x11/common/kwinxrenderutils.h"

#include <xcb/xfixes.h>

#include <QDebug>
#include <QPainter>
#include <QtMath>

namespace KWin
{

// ScreenPaintData WorkspaceSceneXRender::screen_paint;

#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))
#define FIXED_TO_DOUBLE(f) ((double) ((f) / 65536.0))

//****************************************
// WorkspaceSceneXRender
//****************************************

WorkspaceSceneXRender::WorkspaceSceneXRender(XRenderBackend *backend)
    : WorkspaceScene(std::make_unique<ItemRendererXRender>())
    , m_backend(backend)
{
}

WorkspaceSceneXRender::~WorkspaceSceneXRender()
{
}

std::unique_ptr<Shadow> WorkspaceSceneXRender::createShadow(Window *window)
{
    return std::make_unique<SceneXRenderShadow>(window);
}

DecorationRenderer *WorkspaceSceneXRender::createDecorationRenderer(Decoration::DecoratedClientImpl *impl)
{
    return new SceneXRenderDecorationRenderer(impl);
}


xcb_render_picture_t WorkspaceSceneXRender::xrenderBufferPicture() const
{
    return m_backend->buffer();
}


//****************************************
// XRenderShadow
//****************************************
SceneXRenderShadow::SceneXRenderShadow(Window *window)
    : Shadow(window)
{
}

SceneXRenderShadow::~SceneXRenderShadow()
{
}

bool SceneXRenderShadow::prepareBackend()
{
    return true;
}

void SceneXRenderShadow::resetTexture()
{
}

//****************************************
// XRenderDecorationRenderer
//****************************************
SceneXRenderDecorationRenderer::SceneXRenderDecorationRenderer(Decoration::DecoratedClientImpl *client)
    : DecorationRenderer(client)
    , m_gc(XCB_NONE)
{
    for (int i = 0; i < int(DecorationPart::Count); ++i) {
        m_pixmaps[i] = XCB_PIXMAP_NONE;
        m_pictures[i] = nullptr;
    }
}

SceneXRenderDecorationRenderer::~SceneXRenderDecorationRenderer()
{
    for (int i = 0; i < int(DecorationPart::Count); ++i) {
        if (m_pixmaps[i] != XCB_PIXMAP_NONE) {
            xcb_free_pixmap(connection(), m_pixmaps[i]);
        }
        delete m_pictures[i];
    }
    if (m_gc != 0) {
        xcb_free_gc(connection(), m_gc);
    }
}

xcb_render_picture_t SceneXRenderDecorationRenderer::picture(SceneXRenderDecorationRenderer::DecorationPart part) const
{
    Q_ASSERT(part != DecorationPart::Count);
    XRenderPicture *picture = m_pictures[int(part)];
    if (!picture) {
        return XCB_RENDER_PICTURE_NONE;
    }
    return *picture;
}

void SceneXRenderDecorationRenderer::render(const QRegion &region)
{
    if (areImageSizesDirty()) {
        resizePixmaps();
        resetImageSizesDirty();
    }

    const QRectF top(QPoint(0, 0), m_sizes[int(DecorationPart::Top)]);
    const QRectF left(QPoint(0, top.height()), m_sizes[int(DecorationPart::Left)]);
    const QRectF right(QPoint(top.width() - m_sizes[int(DecorationPart::Right)].width(), top.height()), m_sizes[int(DecorationPart::Right)]);
    const QRectF bottom(QPoint(0, left.y() + left.height()), m_sizes[int(DecorationPart::Bottom)]);

    xcb_connection_t *c = connection();
    if (m_gc == 0) {
        m_gc = xcb_generate_id(connection());
        xcb_create_gc(c, m_gc, m_pixmaps[int(DecorationPart::Top)], 0, nullptr);
    }
    auto renderPart = [this, c](const QRectF &geo, const QPointF &offset, int index) {
        if (!geo.isValid()) {
            return;
        }
        QImage image = renderToImage(geo.toRect());

        Q_ASSERT(image.devicePixelRatio() == 1);
        xcb_put_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, m_pixmaps[index], m_gc,
                      image.width(), image.height(), geo.x() - offset.x(), geo.y() - offset.y(), 0, 32,
                      image.sizeInBytes(), image.constBits());
    };
    const QRectF geometry = region.boundingRect();
    renderPart(left.intersected(geometry),   left.topLeft(),   int(DecorationPart::Left));
    renderPart(top.intersected(geometry),    top.topLeft(),    int(DecorationPart::Top));
    renderPart(right.intersected(geometry),  right.topLeft(),  int(DecorationPart::Right));
    renderPart(bottom.intersected(geometry), bottom.topLeft(), int(DecorationPart::Bottom));
    xcb_flush(c);
}

void SceneXRenderDecorationRenderer::resizePixmaps()
{
    QRectF left, top, right, bottom;
    client()->window()->layoutDecorationRects(left, top, right, bottom);

    xcb_connection_t *c = connection();
    auto checkAndCreate = [this, c](int border, const QRectF &rect) {
        const QSizeF size = rect.size();
        if (m_sizes[border] != size) {
            m_sizes[border] = size;
            if (m_pixmaps[border] != XCB_PIXMAP_NONE) {
                xcb_free_pixmap(c, m_pixmaps[border]);
            }
            delete m_pictures[border];
            if (!size.isEmpty()) {
                m_pixmaps[border] = xcb_generate_id(connection());
                xcb_create_pixmap(connection(), 32, m_pixmaps[border], rootWindow(), size.width(), size.height());
                m_pictures[border] = new XRenderPicture(m_pixmaps[border], 32);
            } else {
                m_pixmaps[border] = XCB_PIXMAP_NONE;
                m_pictures[border] = nullptr;
            }
        }
        if (!m_pictures[border]) {
            return;
        }
        // fill transparent
        xcb_rectangle_t r = {0, 0, uint16_t(size.width()), uint16_t(size.height())};
        xcb_render_fill_rectangles(connection(), XCB_RENDER_PICT_OP_SRC, *m_pictures[border], XRenderUtils::preMultiply(Qt::transparent), 1, &r);
    };

    checkAndCreate(int(DecorationPart::Left), left);
    checkAndCreate(int(DecorationPart::Top), top);
    checkAndCreate(int(DecorationPart::Right), right);
    checkAndCreate(int(DecorationPart::Bottom), bottom);
}

}

/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2006 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWIN_SCENE_XRENDER_H
#define KWIN_SCENE_XRENDER_H

#include "xrenderbackend.h"
#include "scene/decorationitem.h"
#include "scene/workspacescene.h"
#include "shadow.h"
#include "../core/overlaywindow.h"


namespace KWin
{

class XRenderBackend;

#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))
#define FIXED_TO_DOUBLE(f) ((double) ((f) / 65536.0))

class WorkspaceSceneXRender : public WorkspaceScene
{
    Q_OBJECT
public:
    explicit WorkspaceSceneXRender(XRenderBackend *backend);
    ~WorkspaceSceneXRender() override;


    std::unique_ptr<Shadow> createShadow(Window *window) override;
    DecorationRenderer *createDecorationRenderer(Decoration::DecoratedClientImpl *impl) override;

    xcb_render_picture_t xrenderBufferPicture() const override;


    bool animationsSupported() const override {
        return false;
    }

    XRenderBackend *backend() const
    {
        return m_backend;
    }

private:
    XRenderBackend *m_backend;

};


/**
 * @short XRender implementation of Shadow.
 *
 * This class extends Shadow by the elements required for XRender rendering.
 * @author Jacopo De Simoi <wilderkde@gmail.org>
 */

class SceneXRenderShadow : public Shadow
{
public:
    explicit SceneXRenderShadow(Window *window);
    using Shadow::ShadowElements;
    using Shadow::ShadowElementTop;
    using Shadow::ShadowElementTopRight;
    using Shadow::ShadowElementRight;
    using Shadow::ShadowElementBottomRight;
    using Shadow::ShadowElementBottom;
    using Shadow::ShadowElementBottomLeft;
    using Shadow::ShadowElementLeft;
    using Shadow::ShadowElementTopLeft;
    using Shadow::ShadowElementsCount;
    ~SceneXRenderShadow() override;

    void layoutShadowRects(QRect& top, QRect& topRight,
                           QRect& right, QRect& bottomRight,
                           QRect& bottom, QRect& bottomLeft,
                           QRect& Left, QRect& topLeft);
    xcb_render_picture_t picture(ShadowElements element) const;

protected:
    bool prepareBackend() override;
    void resetTexture() override;
private:
    XRenderPicture* m_pictures[ShadowElementsCount];
};

class SceneXRenderDecorationRenderer : public DecorationRenderer
{
    Q_OBJECT
public:
    enum class DecorationPart : int {
        Left,
        Top,
        Right,
        Bottom,
        Count
    };
    explicit SceneXRenderDecorationRenderer(Decoration::DecoratedClientImpl *client);
    ~SceneXRenderDecorationRenderer() override;

    void render(const QRegion &region) override;

    xcb_render_picture_t picture(SceneXRenderDecorationRenderer::DecorationPart part) const;

    void resizePixmaps();

private:
    QSizeF m_sizes[int(DecorationPart::Count)];
    xcb_pixmap_t m_pixmaps[int(DecorationPart::Count)];
    xcb_gcontext_t m_gc;
    XRenderPicture* m_pictures[int(DecorationPart::Count)];
};

} // namespace

// #endif

#endif

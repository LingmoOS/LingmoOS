
/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "scene/surfaceitem_x11.h"

#include <xcb/render.h>

namespace KWin
{

class XRenderBackend;

class KWIN_EXPORT XRenderSurfaceTextureX11 : public SurfaceTexture
{
public:
    explicit XRenderSurfaceTextureX11(XRenderBackend *backend, SurfacePixmapX11 *pixmap);
    ~XRenderSurfaceTextureX11() override;

    bool isValid() const override;

    bool create();
    xcb_render_picture_t picture() const;

private:
    SurfacePixmapX11 *m_pixmap;
    XRenderBackend *m_backend;
    xcb_render_picture_t m_picture = XCB_RENDER_PICTURE_NONE;
};

} // namespace KWin
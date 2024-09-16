/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "scene/surfaceitem.h"
#include <xcb/render.h>

namespace KWin
{

class XRenderBackend;

class KWIN_EXPORT XRenderSurfaceTexture : public SurfaceTexture
{
public:
    explicit XRenderSurfaceTexture(XRenderBackend *backend);
    ~XRenderSurfaceTexture() override;

    bool isValid() const override;

    XRenderBackend *backend() const;
    xcb_render_picture_t picture() const;

    virtual bool create() = 0;
    virtual void update(const QRegion &region) = 0;

protected:
    XRenderBackend *m_backend;
    xcb_render_picture_t m_picture = XCB_RENDER_PICTURE_NONE;
};

} // namespace KWin

/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xrendersurfacetexture.h"

namespace KWin
{

XRenderSurfaceTexture::XRenderSurfaceTexture(XRenderBackend *backend)
    : m_backend(backend)
{
}

XRenderSurfaceTexture::~XRenderSurfaceTexture()
{
}

bool XRenderSurfaceTexture::isValid() const
{
    return m_picture != XCB_RENDER_PICTURE_NONE;
}

XRenderBackend *XRenderSurfaceTexture::backend() const
{
    return m_backend;
}

xcb_render_picture_t XRenderSurfaceTexture::picture() const
{
    return m_picture;
}

} // namespace KWin

/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2006 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2009 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xrenderbackend.h"
#include "utils/xcbutils.h"

namespace KWin
{

XRenderBackend::XRenderBackend()
    : m_buffer(XCB_RENDER_PICTURE_NONE)
    , m_failed(false)
{
    if (!Xcb::Extensions::self()->isRenderAvailable()) {
        setFailed("No XRender extension available");
        return;
    }
    if (!Xcb::Extensions::self()->isFixesRegionAvailable()) {
        setFailed("No XFixes v3+ extension available");
        return;
    }
}

XRenderBackend::~XRenderBackend()
{
    if (m_buffer) {
        xcb_render_free_picture(connection(), m_buffer);
    }
}

CompositingType XRenderBackend::compositingType() const
{
    return XRenderCompositing;
}

std::unique_ptr<SurfaceTexture> XRenderBackend::createSurfaceTextureInternal(SurfacePixmapInternal *pixmap)
{
    return nullptr;
}

std::unique_ptr<SurfaceTexture> XRenderBackend::createSurfaceTextureX11(SurfacePixmapX11 *pixmap)
{
    return std::make_unique<XRenderSurfaceTextureX11>(this, pixmap);
}

OverlayWindow *XRenderBackend::overlayWindow() const
{
    return nullptr;
}

void XRenderBackend::showOverlay()
{
}

xcb_render_picture_t XRenderBackend::buffer() const
{
    return m_buffer;
}

void XRenderBackend::setBuffer(xcb_render_picture_t buffer)
{
    if (m_buffer != XCB_RENDER_PICTURE_NONE) {
        xcb_render_free_picture(connection(), m_buffer);
    }
    m_buffer = buffer;
}

bool XRenderBackend::isFailed() const
{
    return m_failed;
}

void XRenderBackend::setFailed(const QString &reason)
{
    m_failed = true;
}

void XRenderBackend::screenGeometryChanged()
{
}


} // namespace KWin

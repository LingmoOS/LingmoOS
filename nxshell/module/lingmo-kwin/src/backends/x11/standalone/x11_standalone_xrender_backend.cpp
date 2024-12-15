/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2006 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2009 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_standalone_xrender_backend.h"
#include "xrenderbackend.h"
#include "main.h"
#include "x11_standalone_backend.h"
#include "x11_standalone_overlaywindow.h"
#include "core/renderloop_p.h"
#include "scene/workspacescene.h"
#include "softwarevsyncmonitor.h"
#include "utils/common.h"
#include "utils/xcbutils.h"
#include "workspace.h"
#include "composite.h"

#include "../common/kwinxrenderutils.h"

#include <xcb/xfixes.h>

#include <QRegion>
namespace KWin
{

XRenderLayer::XRenderLayer(X11XRenderBackend *backend)
    : m_backend(backend)
{
}

std::optional<OutputLayerBeginFrameInfo> XRenderLayer::beginFrame()
{
    return m_backend->beginFrame();
}

bool XRenderLayer::endFrame(const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    m_backend->endFrame(renderedRegion, damagedRegion);
    return true;
}

X11XRenderBackend::X11XRenderBackend(X11StandaloneBackend *backend)
    : XRenderBackend()
    , m_backend(backend)
    , m_vsyncMonitor(SoftwareVsyncMonitor::create())
    , m_overlayWindow(new OverlayWindowX11)
    , m_front(XCB_RENDER_PICTURE_NONE)
    , m_format(0)
    , m_layer(std::make_unique<XRenderLayer>(this))
{
    // Fallback to software vblank events for now. Maybe use the Present extension or
    // something to get notified when the overlay window is actually presented?
    m_vsyncMonitor = SoftwareVsyncMonitor::create();
    connect(backend->renderLoop(), &RenderLoop::refreshRateChanged, this, [this, backend]() {
        m_vsyncMonitor->setRefreshRate(backend->renderLoop()->refreshRate());
    });

    m_vsyncMonitor->setRefreshRate(backend->renderLoop()->refreshRate());

    connect(m_vsyncMonitor.get(), &VsyncMonitor::vblankOccurred, this, &X11XRenderBackend::vblank);

    init(true);

    connect(workspace(), &Workspace::geometryChanged, this, &X11XRenderBackend::screenGeometryChanged);
}

X11XRenderBackend::~X11XRenderBackend()
{
    // No completion events will be received for in-flight frames, this may lock the
    // render loop. We need to ensure that the render loop is back to its initial state
    // if the render backend is about to be destroyed.
    const auto platform = static_cast<X11StandaloneBackend *>(kwinApp()->outputBackend());
    RenderLoopPrivate::get(platform->renderLoop())->invalidate();

    if (m_front) {
        xcb_render_free_picture(connection(), m_front);
    }
    m_overlayWindow->destroy();
}

OverlayWindow *X11XRenderBackend::overlayWindow() const
{
    return m_overlayWindow.get();
}

void X11XRenderBackend::showOverlay()
{
    if (m_overlayWindow->window()) { // show the window only after the first pass, since
        m_overlayWindow->show();   // that pass may take long
    }
}

void X11XRenderBackend::init(bool createOverlay)
{
    if (m_front != XCB_RENDER_PICTURE_NONE)
        xcb_render_free_picture(connection(), m_front);
    bool haveOverlay = createOverlay ? m_overlayWindow->create() : (m_overlayWindow->window() != XCB_WINDOW_NONE);
    if (haveOverlay) {
        m_overlayWindow->setup(XCB_WINDOW_NONE);
        std::unique_ptr<xcb_get_window_attributes_reply_t> attribs(xcb_get_window_attributes_reply(connection(),
            xcb_get_window_attributes_unchecked(connection(), m_overlayWindow->window()), nullptr));
        if (!attribs) {
            setFailed("Failed getting window attributes for overlay window");
            return;
        }
        m_format = XRenderUtils::findPictFormat(attribs->visual);
        if (m_format == 0) {
            setFailed("Failed to find XRender format for overlay window");
            return;
        }
        m_front = xcb_generate_id(connection());
        xcb_render_create_picture(connection(), m_front, m_overlayWindow->window(), m_format, 0, nullptr);
            } else {
        // create XRender picture for the root window
        m_format = XRenderUtils::findPictFormat(Xcb::defaultScreen()->root_visual);
        if (m_format == 0) {
            setFailed("Failed to find XRender format for root window");
            return; // error
        }
        m_front = xcb_generate_id(connection());
        const uint32_t values[] = {XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS};
        xcb_render_create_picture(connection(), m_front, rootWindow(), m_format, XCB_RENDER_CP_SUBWINDOW_MODE, values);
            }
    createBuffer();
}

void X11XRenderBackend::createBuffer()
{
    xcb_pixmap_t pixmap = xcb_generate_id(connection());
    const auto displaySize = workspace()->geometry().size();
    xcb_create_pixmap(connection(), Xcb::defaultDepth(), pixmap, rootWindow(), displaySize.width(), displaySize.height());
    renderPicture = xcb_generate_id(connection());
    xcb_render_create_picture(connection(), renderPicture, pixmap, m_format, 0, nullptr);
    xcb_free_pixmap(connection(), pixmap);   // The picture owns the pixmap now
    setBuffer(renderPicture);
}

OutputLayerBeginFrameInfo X11XRenderBackend::beginFrame()
{
        QRegion repaint = m_layer->repaints();

        return OutputLayerBeginFrameInfo{
        .renderTarget = RenderTarget(&renderPicture),
        .repaint = repaint,
    };

}

void X11XRenderBackend::endFrame(const QRegion &renderedRegion, const QRegion &damagedRegion)
{
}

void X11XRenderBackend::present(Output *output /* int mask, const QRegion &damage */ )
{
    m_vsyncMonitor->arm();

    const auto displaySize = workspace()->geometry().size();
    if (Scene::PAINT_SCREEN_REGION) {
        // Use the damage region as the clip region for the root window
        QRegion region(workspace()->geometry(), QRegion::Rectangle);
        XFixesRegion frontRegion(region);
        xcb_xfixes_set_picture_clip_region(connection(), m_front, frontRegion, 0, 0);
        // copy composed buffer to the root window
        xcb_xfixes_set_picture_clip_region(connection(), buffer(), XCB_XFIXES_REGION_NONE, 0, 0);
        xcb_render_composite(connection(), XCB_RENDER_PICT_OP_SRC, buffer(), XCB_RENDER_PICTURE_NONE,
                             m_front, 0, 0, 0, 0, 0, 0, displaySize.width(), displaySize.height());
        xcb_xfixes_set_picture_clip_region(connection(), m_front, XCB_XFIXES_REGION_NONE, 0, 0);
    } else {
        // copy composed buffer to the root window
        xcb_render_composite(connection(), XCB_RENDER_PICT_OP_SRC, buffer(), XCB_RENDER_PICTURE_NONE,
                             m_front, 0, 0, 0, 0, 0, 0, displaySize.width(), displaySize.height());
    }

    xcb_flush(connection());
}

OutputLayer* X11XRenderBackend::primaryLayer(Output *output)
{
    return m_layer.get();
}

void X11XRenderBackend::vblank(std::chrono::nanoseconds timestamp)
{
    RenderLoopPrivate *renderLoopPrivate = RenderLoopPrivate::get(m_backend->renderLoop());
    renderLoopPrivate->notifyFrameCompleted(timestamp);
}

void X11XRenderBackend::screenGeometryChanged()
{
    init(false);
}

} // namespace KWin

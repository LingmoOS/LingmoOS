/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2006 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "xrenderbackend.h"

#include "core/outputlayer.h"

#include <memory>

namespace KWin
{

class SoftwareVsyncMonitor;
class X11StandaloneBackend;
class X11XRenderBackend;


class XRenderLayer : public OutputLayer
{
public:
    XRenderLayer(X11XRenderBackend *backend);

    std::optional<OutputLayerBeginFrameInfo> beginFrame() override;
    bool endFrame(const QRegion &renderedRegion, const QRegion &damagedRegion) override;

private:
    X11XRenderBackend *const m_backend;
};


/**
 * @brief XRenderBackend using an X11 Overlay Window as compositing target.
 */
class X11XRenderBackend : public XRenderBackend
{
    Q_OBJECT

public:
    explicit X11XRenderBackend(X11StandaloneBackend *backend);
    ~X11XRenderBackend() override;


    OutputLayerBeginFrameInfo beginFrame();
    void endFrame(const QRegion &renderedRegion, const QRegion &damagedRegion);

    void present(Output *output) override;
    OutputLayer *primaryLayer(Output *output) override;

    OverlayWindow *overlayWindow() const override;
    void showOverlay() override;
    void screenGeometryChanged() override;

private:
    void init(bool createOverlay);
    void createBuffer();
    void vblank(std::chrono::nanoseconds timestamp);

    X11StandaloneBackend *m_backend;
    std::unique_ptr<SoftwareVsyncMonitor> m_vsyncMonitor;
    std::unique_ptr<OverlayWindow> m_overlayWindow;
    xcb_render_picture_t m_front;
    xcb_render_pictformat_t m_format;

    std::unique_ptr<XRenderLayer> m_layer;

    xcb_render_picture_t renderPicture;

};

} // namespace KWin

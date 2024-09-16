// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsglobal.h"
#include "dlayershellwindow.h"
#include "qwaylandlayershellsurface_p.h"

#include <qwayland-wlr-layer-shell-unstable-v1.h>
#include <wayland-wlr-layer-shell-unstable-v1-client-protocol.h>

#include <QLoggingCategory>

#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

Q_LOGGING_CATEGORY(layershellsurface, "dde.shell.layershell.surface")

DS_BEGIN_NAMESPACE

QWaylandLayerShellSurface::QWaylandLayerShellSurface(QtWayland::zwlr_layer_shell_v1 *shell, QtWaylandClient::QWaylandWindow *window)
    : QtWaylandClient::QWaylandShellSurface(window)
    , QtWayland::zwlr_layer_surface_v1()
    , m_dlayerShellWindow(DLayerShellWindow::get(window->window()))
{

    wl_output *output = nullptr;
    if (m_dlayerShellWindow->screenConfiguration() == DLayerShellWindow::ScreenFromQWindow) {
        auto waylandScreen = dynamic_cast<QtWaylandClient::QWaylandScreen*>(window->window()->screen()->handle());
        connect(window->window(), &QWindow::screenChanged, this, [window](){
            window->reset();
            window->reinit();
        });
        if (!waylandScreen) {
            qCWarning(layershellsurface) << "failed to get screen for wayland";
        } else {
            output = waylandScreen->output();
        }
    }

    init(shell->get_layer_surface(window->waylandSurface()->object(), output, m_dlayerShellWindow->layer(), m_dlayerShellWindow->scope()));

    set_layer(m_dlayerShellWindow->layer());
    connect(m_dlayerShellWindow, &DLayerShellWindow::layerChanged, this, [this, window](){
        set_layer(m_dlayerShellWindow->layer());
        window->waylandSurface()->commit();
    });

    set_anchor(m_dlayerShellWindow->anchors());
    connect(m_dlayerShellWindow, &DLayerShellWindow::anchorsChanged, this,[this, window](){
        trySetAnchorsAndSize();
    });

    set_exclusive_zone(m_dlayerShellWindow->exclusionZone());
    connect(m_dlayerShellWindow, &DLayerShellWindow::exclusionZoneChanged, this,[this, window](){
        set_exclusive_zone(m_dlayerShellWindow->exclusionZone());
        window->waylandSurface()->commit();
    });

    set_margin(m_dlayerShellWindow->topMargin(), m_dlayerShellWindow->rightMargin(), m_dlayerShellWindow->bottomMargin(), m_dlayerShellWindow->leftMargin());
    connect(m_dlayerShellWindow, &DLayerShellWindow::marginsChanged, this, [this](){
        set_margin(m_dlayerShellWindow->topMargin(), m_dlayerShellWindow->rightMargin(), m_dlayerShellWindow->bottomMargin(), m_dlayerShellWindow->leftMargin());
    });

    set_keyboard_interactivity(m_dlayerShellWindow->keyboardInteractivity());
    connect(m_dlayerShellWindow, &DLayerShellWindow::keyboardInteractivityChanged, this, [this, window](){
        set_keyboard_interactivity(m_dlayerShellWindow->keyboardInteractivity());
        window->waylandSurface()->commit();
    });

    calcAndSetRequestSize(window->surfaceSize());

    if (m_requestSize.isValid()) {
        set_size(m_requestSize.width(), m_requestSize.height());
    }
}

QWaylandLayerShellSurface::~QWaylandLayerShellSurface()
{
    destroy();
}

void QWaylandLayerShellSurface::zwlr_layer_surface_v1_closed()
{
    if (m_dlayerShellWindow->closeOnDismissed()) {
        window()->window()->close();
    }
}

void QWaylandLayerShellSurface::calcAndSetRequestSize(QSize requestSize)
{
    auto anchors = m_dlayerShellWindow->anchors();
    const bool horizontallyConstrained = anchors.testFlags({DLayerShellWindow::AnchorLeft, DLayerShellWindow::AnchorRight});
    const bool verticallyConstrained = anchors.testFlags({DLayerShellWindow::AnchorTop, DLayerShellWindow::AnchorBottom});
    m_requestSize = requestSize;
    if (horizontallyConstrained) {
        m_requestSize.setWidth(0);
    }
    if (verticallyConstrained) {
        m_requestSize.setHeight(0);
    }
}

bool QWaylandLayerShellSurface::anchorsSizeConflict() const
{
    auto anchors = m_dlayerShellWindow->anchors();
    const bool horizontallyConstrained = anchors.testFlags({DLayerShellWindow::AnchorLeft, DLayerShellWindow::AnchorRight});
    const bool verticallyConstrained = anchors.testFlags({DLayerShellWindow::AnchorTop, DLayerShellWindow::AnchorBottom});
    return (!horizontallyConstrained && m_requestSize.width() == 0) || (!verticallyConstrained && m_requestSize.height() == 0);
}

void QWaylandLayerShellSurface::trySetAnchorsAndSize()
{
    if (!anchorsSizeConflict()) {
        set_anchor(m_dlayerShellWindow->anchors());
        set_size(m_requestSize.width(), m_requestSize.height());
        window()->waylandSurface()->commit();
    }
}

void QWaylandLayerShellSurface::zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height)
{
    ack_configure(serial);
    m_pendingSize = QSize(width, height);

    if (!m_configured) {
        m_configured = true;
        window()->resizeFromApplyConfigure(m_pendingSize);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
        window()->handleExpose(QRect(QPoint(), m_pendingSize));
#else
        window()->sendRecursiveExposeEvent();
#endif

    } else {
        window()->applyConfigureWhenPossible();
    }
}

void QWaylandLayerShellSurface::applyConfigure()
{
    window()->resizeFromApplyConfigure(m_pendingSize);
}

void QWaylandLayerShellSurface::setWindowGeometry(const QRect &geometry)
{
    calcAndSetRequestSize(geometry.size());
    trySetAnchorsAndSize();
}

void QWaylandLayerShellSurface::attachPopup(QtWaylandClient::QWaylandShellSurface *popup)
{
    std::any anyRole = popup->surfaceRole();

    if (auto role = std::any_cast<::xdg_popup *>(&anyRole)) {
        get_popup(*role);
    } else {
        qCWarning(layershellsurface) << "Cannot attach popup of unknown type";
    }
}

DS_END_NAMESPACE

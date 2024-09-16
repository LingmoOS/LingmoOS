// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcontextshellwindow.h"
#include "qwaylandwindowcontextshellsurface_p.h"
#include "wayland/qwaylandpersonalizationshellintegration_p.h"

#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

Q_LOGGING_CATEGORY(layershellsurface, "dde.shell.layershell.surface")

DGUI_BEGIN_NAMESPACE
QWaylandWindowContextSurface::QWaylandWindowContextSurface(
    QWaylandPersonalizationShellIntegration *shell, QtWaylandClient::QWaylandWindow *window)
    : QtWaylandClient::QWaylandShellSurface(window)
    , QtWayland::personalization_window_context_v1()
    , m_dcontextShellWindow(DContextShellWindow::get(window->window()))
{
    init(shell->get_window_context(window->waylandSurface()->object()));
    auto onNoTitlebarChanged = [this, window] {
        if (m_dcontextShellWindow->noTitlebar()) {
            set_no_titlebar(PERSONALIZATION_WINDOW_CONTEXT_V1_ENABLE_MODE_ENABLE);
        } else {
            set_no_titlebar(PERSONALIZATION_WINDOW_CONTEXT_V1_ENABLE_MODE_DISABLE);
        }
        window->waylandSurface()->commit();
    };

    connect(m_dcontextShellWindow,
            &DContextShellWindow::noTitlebarChanged,
            this,
            onNoTitlebarChanged);

    onNoTitlebarChanged();
}

QWaylandWindowContextSurface::~QWaylandWindowContextSurface()
{
    destroy();
}
DGUI_END_NAMESPACE

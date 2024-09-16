// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dcontextshellwindow.h"

#include <dtkgui_global.h>
#include <private/qwaylandwindow_p.h>
#include <qwayland-treeland-personalization-manager-v1.h>

#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

DGUI_BEGIN_NAMESPACE
class QWaylandPersonalizationShellIntegration;
class QWaylandWindowContextSurface : public QtWaylandClient::QWaylandShellSurface,
                                     public QtWayland::personalization_window_context_v1
{
    Q_OBJECT
public:
    QWaylandWindowContextSurface(QWaylandPersonalizationShellIntegration *shell,
                                 QtWaylandClient::QWaylandWindow *window);
    ~QWaylandWindowContextSurface() override;

private:
    DContextShellWindow *m_dcontextShellWindow;
};
DGUI_END_NAMESPACE

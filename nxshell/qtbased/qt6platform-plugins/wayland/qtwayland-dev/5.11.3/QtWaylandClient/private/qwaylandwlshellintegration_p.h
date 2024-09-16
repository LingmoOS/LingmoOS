// Copyright (C) 2016 Jolla Ltd
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDWLSHELLINTEGRATION_P_H
#define QWAYLANDWLSHELLINTEGRATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <wayland-client.h>
#include <private/qwayland-wayland.h>

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class Q_WAYLAND_CLIENT_EXPORT QWaylandWlShellIntegration : public QWaylandShellIntegration
{
public:
    static QWaylandWlShellIntegration *create(QWaylandDisplay* display);
    bool initialize(QWaylandDisplay *) override;
    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;

private:
    QWaylandWlShellIntegration(QWaylandDisplay* display);

    QtWayland::wl_shell *m_wlShell = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDWLSHELLINTEGRATION_P_H

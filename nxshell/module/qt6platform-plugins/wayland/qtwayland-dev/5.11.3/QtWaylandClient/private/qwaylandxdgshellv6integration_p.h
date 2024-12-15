// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDXDGSHELLV6INTEGRATION_P_H
#define QWAYLANDXDGSHELLV6INTEGRATION_P_H

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

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>

    QT_BEGIN_NAMESPACE

    namespace QtWaylandClient {

class QWaylandXdgShellV6;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgShellV6Integration : public QWaylandShellIntegration
{
public:
    static QWaylandXdgShellV6Integration *create(QWaylandDisplay* display);
    bool initialize(QWaylandDisplay *display) override;
    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;

private:
    QWaylandXdgShellV6Integration(QWaylandDisplay *display);

    QWaylandXdgShellV6 *m_xdgShell = nullptr;
};
}

QT_END_NAMESPACE

#endif // QWAYLANDXDGSHELLV6INTEGRATION_P_H

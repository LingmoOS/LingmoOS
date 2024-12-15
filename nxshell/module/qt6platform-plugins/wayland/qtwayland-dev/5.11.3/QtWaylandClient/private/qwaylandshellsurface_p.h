// Copyright (C) 2021 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDSHELLSURFACE_H
#define QWAYLANDSHELLSURFACE_H

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

#include <QtCore/QSize>
#include <QObject>

#include <wayland-client.h>

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

class QVariant;
class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandInputDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShellSurface : public QObject
{
    Q_OBJECT
public:
    explicit QWaylandShellSurface(QWaylandWindow *window);
    ~QWaylandShellSurface() override {}
    virtual void resize(QWaylandInputDevice * /*inputDevice*/, enum wl_shell_surface_resize /*edges*/)
    {}

    virtual bool move(QWaylandInputDevice *) { return false; }
    virtual void setTitle(const QString & /*title*/) {}
    virtual void setAppId(const QString & /*appId*/) {}

    virtual void setWindowFlags(Qt::WindowFlags flags);

    virtual bool isExposed() const { return true; }
    virtual bool handleExpose(const QRegion &) { return false; }

    virtual void raise() {}
    virtual void lower() {}
    virtual void setContentOrientationMask(Qt::ScreenOrientations orientation) { Q_UNUSED(orientation) }

    virtual void sendProperty(const QString &name, const QVariant &value);

    inline QWaylandWindow *window() { return m_window; }

    virtual void applyConfigure() {}
    virtual void requestWindowStates(Qt::WindowStates states) {Q_UNUSED(states);}
    virtual bool wantsDecorations() const { return false; }

private:
    QWaylandWindow *m_window = nullptr;
    friend class QWaylandWindow;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDSHELLSURFACE_H

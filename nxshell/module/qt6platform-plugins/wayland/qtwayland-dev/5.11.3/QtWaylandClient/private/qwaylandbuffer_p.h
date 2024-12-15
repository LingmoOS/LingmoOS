// Copyright (C) 2018 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDBUFFER_H
#define QWAYLANDBUFFER_H

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

#include <QtWaylandClient/qtwaylandclientglobal.h>

#include <QtCore/QSize>
#include <QtCore/QRect>

#include <wayland-client.h>
#include <wayland-client-protocol.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class Q_WAYLAND_CLIENT_EXPORT QWaylandBuffer {
public:
    QWaylandBuffer();
    virtual ~QWaylandBuffer();
    void init(wl_buffer *buf);

    wl_buffer *buffer() {return mBuffer;}
    virtual QSize size() const = 0;
    virtual int scale() const { return 1; }

    void setBusy() { mBusy = true; }
    bool busy() const { return mBusy; }

    void setCommitted() { mCommitted = true; }
    bool committed() const { return mCommitted; }

protected:
    struct wl_buffer *mBuffer = nullptr;

private:
    bool mBusy = false;
    bool mCommitted = false;

    static void release(void *data, wl_buffer *);
    static const wl_buffer_listener listener;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDBUFFER_H

/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_BUFFER_P_H
#define WAYLAND_BUFFER_P_H
#include "buffer.h"
#include "wayland_pointer_p.h"
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Buffer::Private
{
public:
    Private(Buffer *q, ShmPool *parent, wl_buffer *nativeBuffer, const QSize &size, int32_t stride, size_t offset, Format format);
    ~Private();
    void destroy();

    ShmPool *shm;
    WaylandPointer<wl_buffer, wl_buffer_destroy> nativeBuffer;
    bool released;
    QSize size;
    int32_t stride;
    size_t offset;
    bool used;
    Format format;

private:
    Buffer *q;
    static const struct wl_buffer_listener s_listener;
    static void releasedCallback(void *data, wl_buffer *wl_buffer);
};

}
}

#endif

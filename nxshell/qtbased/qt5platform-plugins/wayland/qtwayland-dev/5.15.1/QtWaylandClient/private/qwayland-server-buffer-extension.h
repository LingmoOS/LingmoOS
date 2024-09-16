// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QT_WAYLAND_SERVER_BUFFER_EXTENSION
#define QT_WAYLAND_SERVER_BUFFER_EXTENSION

#include <QtWaylandClient/private/wayland-server-buffer-extension-client-protocol.h>
#include <QByteArray>
#include <QString>

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")

#if !defined(Q_WAYLAND_CLIENT_SERVER_BUFFER_EXTENSION_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_CLIENT_SERVER_BUFFER_EXTENSION_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_CLIENT_SERVER_BUFFER_EXTENSION_EXPORT
#  endif
#endif

namespace QtWayland {
    class Q_WAYLAND_CLIENT_SERVER_BUFFER_EXTENSION_EXPORT qt_server_buffer
    {
    public:
        qt_server_buffer(struct ::wl_registry *registry, int id, int version);
        qt_server_buffer(struct ::qt_server_buffer *object);
        qt_server_buffer();

        virtual ~qt_server_buffer();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::qt_server_buffer *object);

        struct ::qt_server_buffer *object() { return m_qt_server_buffer; }
        const struct ::qt_server_buffer *object() const { return m_qt_server_buffer; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void release();

    private:
        struct ::qt_server_buffer *m_qt_server_buffer;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif

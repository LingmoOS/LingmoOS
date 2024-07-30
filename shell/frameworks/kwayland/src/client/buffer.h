/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_BUFFER_H
#define WAYLAND_BUFFER_H

#include <QScopedPointer>
#include <QSize>
#include <QWeakPointer>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_buffer;

namespace KWayland
{
namespace Client
{
class ShmPool;

/**
 * @short Wrapper class for wl_buffer interface.
 *
 * The Buffer is provided by ShmPool and is owned by ShmPool.
 *
 * @see ShmPool
 **/
class KWAYLANDCLIENT_EXPORT Buffer
{
public:
    /**
     * All image formats supported by the implementation.
     **/
    enum class Format {
        ARGB32, ///< 32-bit ARGB format, can be used for QImage::Format_ARGB32 and QImage::Format_ARGB32_Premultiplied
        RGB32, ///< 32-bit RGB format, can be used for QImage::Format_RGB32
    };

    ~Buffer();
    /**
     * Copies the data from @p src into the Buffer.
     **/
    void copy(const void *src);
    /**
     * Sets the Buffer as @p released.
     * This is automatically invoked when the Wayland server sends the release event.
     * @param released Whether the Buffer got released by the Wayland server.
     * @see isReleased
     **/
    void setReleased(bool released);
    /**
     * Sets whether the Buffer is used.
     * If the Buffer may not be reused when it gets released, the user of a Buffer should
     * mark the Buffer as used. This is needed for example when the memory is shared with
     * a QImage. As soon as the Buffer can be reused again one should call this method with
     * @c false again.
     *
     * By default a Buffer is not used.
     *
     * @param used Whether the Buffer should be marked as used.
     * @see isUsed
     **/
    void setUsed(bool used);

    wl_buffer *buffer() const;
    /**
     * @returns The size of this Buffer.
     **/
    QSize size() const;
    /**
     * @returns The stride (bytes per line) of this Buffer.
     **/
    int32_t stride() const;
    /**
     * @returns @c true if the Wayland server doesn't need the Buffer anymore.
     **/
    bool isReleased() const;
    /**
     * @returns @c true if the Buffer's user is still needing the Buffer.
     **/
    bool isUsed() const;
    /**
     * @returns the memory address of this Buffer.
     **/
    uchar *address();
    /**
     * @returns The image format used by this Buffer.
     **/
    Format format() const;

    operator wl_buffer *();
    operator wl_buffer *() const;

    typedef QWeakPointer<Buffer> Ptr;

    /**
     * Helper method to get the id for a provided native buffer.
     * @since 5.3
     **/
    static quint32 getId(wl_buffer *b);

private:
    friend class ShmPool;
    explicit Buffer(ShmPool *parent, wl_buffer *buffer, const QSize &size, int32_t stride, size_t offset, Format format);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif

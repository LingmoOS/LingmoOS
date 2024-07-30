/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SHM_POOL_H
#define WAYLAND_SHM_POOL_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"
#include "buffer.h"

class QImage;
class QSize;

struct wl_shm;

namespace KWayland
{
namespace Client
{
class EventQueue;

/**
 * @short Wrapper class for wl_shm interface.
 *
 * This class holds a shared memory pool together with the Wayland server.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create a ShmPool instance:
 * @code
 * ShmPool *s = registry->createShmPool(name, version);
 * @endcode
 *
 * This creates the ShmPool and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * ShmPool *s = new ShmPool;
 * s->setup(registry->bindShm(name, version));
 * @endcode
 *
 * The ShmPool holds a memory-mapped file from which it provides Buffers.
 * All Buffers are held by the ShmPool and can be reused. Whenever a Buffer
 * is requested the ShmPool tries to reuse an existing Buffer. A Buffer can
 * be reused if the following conditions hold
 * @li it's no longer marked as used
 * @li the server released the buffer
 * @li the size matches
 * @li the stride matches
 * @li the format matches
 *
 * The ownership of a Buffer stays with ShmPool. The ShmPool might destroy the
 * Buffer at any given time. Because of that ShmPool only provides QWeakPointer
 * for Buffers. Users should always check whether the pointer is still valid and
 * only promote to a QSharedPointer for a short time, e.g. to set new data.
 *
 * The ShmPool can provide Buffers for different purposes. One can create a Buffer
 * from an existing QImage. This will use a Buffer with same size, stride and image
 * format as the QImage and <b>copy</b> the content of the QImage into the Buffer.
 * The memory is <b>not</b> shared:
 * @code
 * QImage image(24, 24, QImage::Format_ARG32);
 * image.fill(Qt::transparent);
 * Buffer::Ptr buffer = s->createBuffer(image);
 * @endcode
 *
 * It is also possible to create a Buffer and copy the content from a generic location.
 * Like above this doesn't share the content but copies it:
 * @code
 * QImage image(24, 24, QImage::Format_ARG32);
 * image.fill(Qt::transparent);
 * Buffer::Ptr buffer = s->createBuffer(image.size(), image.bytesPerLine(), image.constBits());
 * @endcode
 *
 * Last but not least it is possible to get a Buffer without copying content directly to it.
 * This means an empty area is just reserved and can be used to e.g. share the memory with a
 * QImage:
 * @code
 * const QSize size = QSize(24, 24);
 * const int stride = size.width() * 4;
 * Buffer::Ptr buffer = s->getBuffer(size, stride, Buffer::Format::RGB32);
 * if (!buffer) {
 *     qDebug() << "Didn't get a valid Buffer";
 *     return;
 * }
 * QImage image(buffer.toStrongRef()->address(), size.width(), size.height(), stride, QImage::Format_RGB32);
 * image.fill(Qt::black);
 * @endcode
 *
 * A Buffer can be attached to a Surface:
 * @code
 * Compositor *c = registry.createCompositor(name, version);
 * Surface *s = c->createSurface();
 * s->attachBuffer(buffer);
 * s->damage(QRect(QPoint(0, 0), size));
 * @endcode
 *
 * Once a Buffer is attached to a Surface and the Surface is committed, it might be released
 * by the Wayland server and thus is free to be reused again. If the client code wants to
 * continue using the Buffer it must call Buffer::setUsed on it. This is important if the memory
 * is shared for example with a QImage as the memory buffer for a QImage must remain valid
 * throughout the life time of the QImage:
 * @code
 * buffer.toStrongRef()->setUsed(true);
 * @endcode
 *
 * This is also important for the case that the shared memory pool needs to be resized.
 * The ShmPool will automatically resize if it cannot provide a new Buffer. During the resize
 * all existing Buffers are unmapped and any shared objects must be recreated. The ShmPool emits
 * the signal poolResized() after the pool got resized.
 *
 * @see Buffer
 **/
class KWAYLANDCLIENT_EXPORT ShmPool : public QObject
{
    Q_OBJECT
public:
    explicit ShmPool(QObject *parent = nullptr);
    ~ShmPool() override;
    /**
     * @returns @c true if the ShmPool references a wl_shm interface and the shared memory pool
     * is setup.
     **/
    bool isValid() const;
    /**
     * Setup this ShmPool to manage the @p shm.
     * This also creates the shared memory pool.
     * When using Registry::createShmPool there is no need to call this
     * method.
     **/
    void setup(wl_shm *shm);
    /**
     * Releases the wl_shm interface.
     * After the interface has been released the ShmPool instance is no
     * longer valid and can be setup with another wl_shm interface.
     *
     * This also destroys the shared memory pool and all Buffers are destroyed.
     **/
    void release();
    /**
     * Destroys the data held by this ShmPool.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_shm interface
     * once there is a new connection available.
     *
     * All Buffers are destroyed!
     *
     * This method is automatically invoked when the Registry which created this
     * ShmPool gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Buffer.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Buffer.
     **/
    EventQueue *eventQueue();

    /**
     * Provides a Buffer with:
     * @li same size as @p image
     * @li same stride as @p image
     * @li same format as @p image
     *
     * If the ShmPool fails to provide such a Buffer a @c null Buffer::Ptr is returned.
     * The content of the @p image is <b>copied</b> into the buffer. The @p image and
     * returned Buffer do <b>not</b> share memory.
     *
     * @param image The image which should be copied into the Buffer
     * @return Buffer with copied content of @p image in success case, a @c null Buffer::Ptr otherwise
     * @see getBuffer
     **/
    Buffer::Ptr createBuffer(const QImage &image);
    /**
     * Provides a Buffer with @p size, @p stride and @p format.
     *
     * If the ShmPool fails to provide such a Buffer a @c null Buffer::Ptr is returned.
     * A memory copy is performed from @p src into the Buffer. The Buffer does <b>not</b> share
     * memory with @p src.
     *
     * @param size The requested size for the Buffer
     * @param stride The requested stride for the Buffer
     * @param src The source memory location to copy from
     * @param format The requested format for the Buffer
     * @return Buffer with copied content of @p src in success case, a @c null Buffer::Ptr otherwise
     * @see getBuffer
     **/
    Buffer::Ptr createBuffer(const QSize &size, int32_t stride, const void *src, Buffer::Format format = Buffer::Format::ARGB32);
    void *poolAddress() const;
    /**
     * Provides a Buffer with @p size, @p stride and @p format.
     *
     * If the ShmPool fails to provide such a Buffer a @c null Buffer::Ptr is returned.
     * Unlike with createBuffer there is no memory copy performed. This provides a bare Buffer
     * to be used by the user.
     *
     * @param size The requested size for the Buffer
     * @param stride The requested stride for the Buffer
     * @param format The requested format for the Buffer
     * @return Buffer as requested in success case, a @c null Buffer::Ptr otherwise.
     * @see createBuffer
     **/
    Buffer::Ptr getBuffer(const QSize &size, int32_t stride, Buffer::Format format = Buffer::Format::ARGB32);
    wl_shm *shm();
Q_SIGNALS:
    /**
     * This signal is emitted whenever the shared memory pool gets resized.
     * Any used Buffer must be remapped.
     **/
    void poolResized();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createShmPool
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif

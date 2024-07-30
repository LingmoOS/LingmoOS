/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "shm_pool.h"
#include "buffer_p.h"
#include "event_queue.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QImage>
// system
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN ShmPool::Private
{
public:
    Private(ShmPool *q);
    bool createPool();
    bool resizePool(int32_t newSize);
    QList<QSharedPointer<Buffer>>::iterator getBuffer(const QSize &size, int32_t stride, Buffer::Format format);
    WaylandPointer<wl_shm, wl_shm_destroy> shm;
    WaylandPointer<wl_shm_pool, wl_shm_pool_destroy> pool;
    void *poolData = nullptr;
    int fd = -1;
    int32_t size = 1024;
    bool valid = false;
    int offset = 0;
    QList<QSharedPointer<Buffer>> buffers;
    EventQueue *queue = nullptr;

private:
    ShmPool *q;
};

ShmPool::Private::Private(ShmPool *q)
    : q(q)
{
}

ShmPool::ShmPool(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ShmPool::~ShmPool()
{
    release();
}

void ShmPool::release()
{
    d->buffers.clear();
    if (d->poolData) {
        munmap(d->poolData, d->size);
        d->poolData = nullptr;
    }
    if (d->fd != -1) {
        close(d->fd);
        d->fd = -1;
    }
    d->pool.release();
    d->shm.release();
    d->valid = false;
    d->offset = 0;
}

void ShmPool::destroy()
{
    for (auto b : d->buffers) {
        b->d->destroy();
    }
    d->buffers.clear();
    if (d->poolData) {
        munmap(d->poolData, d->size);
        d->poolData = nullptr;
    }
    if (d->fd != -1) {
        close(d->fd);
        d->fd = -1;
    }
    d->pool.destroy();
    d->shm.destroy();
    d->valid = false;
    d->offset = 0;
}

void ShmPool::setup(wl_shm *shm)
{
    Q_ASSERT(shm);
    Q_ASSERT(!d->shm);
    d->shm.setup(shm);
    d->valid = d->createPool();
}

void ShmPool::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *ShmPool::eventQueue()
{
    return d->queue;
}

bool ShmPool::Private::createPool()
{
#if HAVE_MEMFD
    fd = memfd_create("kwayland-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (fd >= 0) {
        fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
    } else
#endif
    {
        char templateName[] = "/tmp/kwayland-shared-XXXXXX";
        fd = mkstemp(templateName);
        if (fd >= 0) {
            unlink(templateName);

            int flags = fcntl(fd, F_GETFD);
            if (flags == -1 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
                close(fd);
                fd = -1;
            }
        }
    }

    if (fd == -1) {
        qCDebug(KWAYLAND_CLIENT) << "Could not open temporary file for Shm pool";
        return false;
    }

    if (ftruncate(fd, size) < 0) {
        qCDebug(KWAYLAND_CLIENT) << "Could not set size for Shm pool file";
        return false;
    }
    poolData = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    pool.setup(wl_shm_create_pool(shm, fd, size));

    if (poolData == MAP_FAILED || !pool) {
        qCDebug(KWAYLAND_CLIENT) << "Creating Shm pool failed";
        return false;
    }
    return true;
}

bool ShmPool::Private::resizePool(int32_t newSize)
{
    if (ftruncate(fd, newSize) < 0) {
        qCDebug(KWAYLAND_CLIENT) << "Could not set new size for Shm pool file";
        return false;
    }
    wl_shm_pool_resize(pool, newSize);
    munmap(poolData, size);
    poolData = mmap(nullptr, newSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    size = newSize;
    if (poolData == MAP_FAILED) {
        qCDebug(KWAYLAND_CLIENT) << "Resizing Shm pool failed";
        return false;
    }
    Q_EMIT q->poolResized();
    return true;
}

namespace
{
static Buffer::Format toBufferFormat(const QImage &image)
{
    switch (image.format()) {
    case QImage::Format_ARGB32_Premultiplied:
        return Buffer::Format::ARGB32;
    case QImage::Format_RGB32:
        return Buffer::Format::RGB32;
    case QImage::Format_ARGB32:
        qCWarning(KWAYLAND_CLIENT) << "Unsupported image format: " << image.format() << ". expect slow performance. Use QImage::Format_ARGB32_Premultiplied";
        return Buffer::Format::ARGB32;
    default:
        qCWarning(KWAYLAND_CLIENT) << "Unsupported image format: " << image.format() << ". expect slow performance.";
        return Buffer::Format::ARGB32;
    }
}
}

Buffer::Ptr ShmPool::createBuffer(const QImage &image)
{
    if (image.isNull() || !d->valid) {
        return QWeakPointer<Buffer>();
    }
    auto format = toBufferFormat(image);
    auto it = d->getBuffer(image.size(), image.bytesPerLine(), format);
    if (it == d->buffers.end()) {
        return QWeakPointer<Buffer>();
    }
    if (format == Buffer::Format::ARGB32 && image.format() != QImage::Format_ARGB32_Premultiplied) {
        auto imageCopy = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        (*it)->copy(imageCopy.bits());
    } else {
        (*it)->copy(image.bits());
    }
    return QWeakPointer<Buffer>(*it);
}

Buffer::Ptr ShmPool::createBuffer(const QSize &size, int32_t stride, const void *src, Buffer::Format format)
{
    if (size.isEmpty() || !d->valid) {
        return QWeakPointer<Buffer>();
    }
    auto it = d->getBuffer(size, stride, format);
    if (it == d->buffers.end()) {
        return QWeakPointer<Buffer>();
    }
    (*it)->copy(src);
    return QWeakPointer<Buffer>(*it);
}

namespace
{
static wl_shm_format toWaylandFormat(Buffer::Format format)
{
    switch (format) {
    case Buffer::Format::ARGB32:
        return WL_SHM_FORMAT_ARGB8888;
    case Buffer::Format::RGB32:
        return WL_SHM_FORMAT_XRGB8888;
    }
    abort();
}
}

Buffer::Ptr ShmPool::getBuffer(const QSize &size, int32_t stride, Buffer::Format format)
{
    auto it = d->getBuffer(size, stride, format);
    if (it == d->buffers.end()) {
        return QWeakPointer<Buffer>();
    }
    return QWeakPointer<Buffer>(*it);
}

QList<QSharedPointer<Buffer>>::iterator ShmPool::Private::getBuffer(const QSize &s, int32_t stride, Buffer::Format format)
{
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
        auto buffer = *it;
        if (!buffer->isReleased() || buffer->isUsed()) {
            continue;
        }
        if (buffer->size() != s || buffer->stride() != stride || buffer->format() != format) {
            continue;
        }
        buffer->setReleased(false);
        return it;
    }
    const int32_t byteCount = s.height() * stride;
    if (offset + byteCount > size) {
        if (!resizePool(size + byteCount)) {
            return buffers.end();
        }
    }
    // we don't have a buffer which we could reuse - need to create a new one
    wl_buffer *native = wl_shm_pool_create_buffer(pool, offset, s.width(), s.height(), stride, toWaylandFormat(format));
    if (!native) {
        return buffers.end();
    }
    if (queue) {
        queue->addProxy(native);
    }
    Buffer *buffer = new Buffer(q, native, s, stride, offset, format);
    offset += byteCount;
    auto it = buffers.insert(buffers.end(), QSharedPointer<Buffer>(buffer));
    return it;
}

bool ShmPool::isValid() const
{
    return d->valid;
}

void *ShmPool::poolAddress() const
{
    return d->poolData;
}

wl_shm *ShmPool::shm()
{
    return d->shm;
}

}
}

#include "moc_shm_pool.cpp"

/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "shm.h"

#include <QGuiApplication>
#include <QImage>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>

static constexpr auto version = 1;

ShmBuffer::ShmBuffer(::wl_buffer *buffer)
    : QtWayland::wl_buffer(buffer)
{
}

ShmBuffer::~ShmBuffer()
{
    destroy();
}

Shm::Shm(QObject *parent)
    : QWaylandClientExtensionTemplate(::version)
{
    setParent(parent);
    connect(this, &QWaylandClientExtension::activeChanged, this, [this] {
        if (!isActive()) {
            wl_shm_destroy(object());
        }
    });
    QMetaObject::invokeMethod(this, "addRegistryListener");
}

Shm *Shm::instance()
{
    static Shm *instance = new Shm(qGuiApp);
    return instance;
}

Shm::~Shm() noexcept
{
    if (isActive()) {
        wl_shm_destroy(object());
    }
}

static wl_shm_format toWaylandFormat(QImage::Format format)
{
    switch (format) {
    case QImage::Format_ARGB32_Premultiplied:
        return WL_SHM_FORMAT_ARGB8888;
    case QImage::Format_RGB32:
        return WL_SHM_FORMAT_XRGB8888;
    case QImage::Format_ARGB32:
        qCWarning(KWAYLAND_KWS()) << "Unsupported image format: " << format << ". expect slow performance. Use QImage::Format_ARGB32_Premultiplied";
        return WL_SHM_FORMAT_ARGB8888;
    default:
        qCWarning(KWAYLAND_KWS()) << "Unsupported image format: " << format << ". expect slow performance.";
        return WL_SHM_FORMAT_ARGB8888;
    }
}

std::unique_ptr<ShmBuffer> Shm::createBuffer(const QImage &image)
{
    if (image.isNull()) {
        return {};
    }
    auto format = toWaylandFormat(image.format());
    const int stride = image.bytesPerLine();
    const int32_t byteCount = image.size().height() * stride;

#if defined HAVE_MEMFD
    int fd = memfd_create("kwayland-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
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
        qCDebug(KWAYLAND_KWS) << "Could not open temporary file for Shm pool";
        return {};
    }

    if (ftruncate(fd, byteCount) < 0) {
        qCDebug(KWAYLAND_KWS) << "Could not set size for Shm pool file";
        close(fd);
        return {};
    }
    auto data = mmap(nullptr, byteCount, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        qCDebug(KWAYLAND_KWS) << "Creating Shm pool failed";
        close(fd);
        return {};
    }

    auto pool = create_pool(fd, byteCount);
    auto *buffer = wl_shm_pool_create_buffer(pool, 0, image.size().width(), image.size().height(), stride, format);
    wl_shm_pool_destroy(pool);

    const QImage &srcImage = [format, &image] {
        if (format == WL_SHM_FORMAT_ARGB8888 && image.format() != QImage::Format_ARGB32_Premultiplied) {
            return image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        } else {
            return image;
        }
    }();

    std::memcpy(static_cast<char *>(data), srcImage.bits(), byteCount);

    munmap(data, byteCount);
    close(fd);
    return std::make_unique<ShmBuffer>(buffer);
}

// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBACKINGSTOREPROXY_H
#define DBACKINGSTOREPROXY_H

#include "global.h"

#include <qpa/qplatformbackingstore.h>

QT_BEGIN_NAMESPACE
class QSharedMemory;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DOpenGLPaintDevice;
class DBackingStoreProxy : public QPlatformBackingStore
{
public:
    static bool useGLPaint(const QWindow *w);
    static bool useWallpaperPaint(const QWindow *w);

    DBackingStoreProxy(QPlatformBackingStore *proxy, bool useGLPaint = false, bool useWallpaper = false);
    ~DBackingStoreProxy() override;

    QPaintDevice *paintDevice() override;

    void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;
#ifndef QT_NO_OPENGL
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context) override;
#elif QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context,
                         bool translucentBackground) override;
#elif QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures,
                         bool translucentBackground) override;
#else
    FlushResult rhiFlush(QWindow *window, qreal sourceDevicePixelRatio, const QRegion &region,
                         const QPoint &offset, QPlatformTextureList *textures,
                         bool translucentBackground) override;
#endif
#if QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
    GLuint toTexture(const QRegion &dirtyRegion, QSize *textureSize, TextureFlags *flags) const override;
#else
    QRhiTexture *toTexture(QRhiResourceUpdateBatch *resourceUpdates, const QRegion &dirtyRegion, TextureFlags *flags) const override;
#endif
#endif
    QImage toImage() const override;
    QPlatformGraphicsBuffer *graphicsBuffer() const override;

    void resize(const QSize &size, const QRegion &staticContents) override;

    bool scroll(const QRegion &area, int dx, int dy) override;

    void beginPaint(const QRegion &region) override;
    void endPaint() override;

private:
    void updateWallpaperShared();

private:
    QPlatformBackingStore *m_proxy = nullptr;
    QImage m_image;
    QRectF m_dirtyWindowRect;
    QRect m_dirtyRect;

    QScopedPointer<DOpenGLPaintDevice> glDevice;
    bool enableGL = false;
    bool enableWallpaper = false;

    QSharedMemory *m_sharedMemory = nullptr;
    QImage m_wallpaper;
};

DPP_END_NAMESPACE

#endif // DBACKINGSTOREPROXY_H

// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbackingstoreproxy.h"
#include "dopenglpaintdevice.h"

#include <QGuiApplication>
#include <QDebug>
#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QSharedMemory>

#include <private/qguiapplication_p.h>
#include <private/qhighdpiscaling_p.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#if __has_include("utility.h")
#define __D_HAS_UTILITY__
#endif
#else
#if QT_HAS_INCLUDE("utility.h")
#define __D_HAS_UTILITY__
#endif
#endif

#ifdef __D_HAS_UTILITY__
#include "utility.h"
#include "dwmsupport.h"
#else
#define DISABLE_WALLPAPER
#endif

#define HEADER_SIZE 16

DPP_BEGIN_NAMESPACE

bool DBackingStoreProxy::useGLPaint(const QWindow *w)
{
#ifndef QT_NO_OPENDL
    if (!w->supportsOpenGL())
        return false;

    if (qEnvironmentVariableIsSet("D_NO_OPENGL") || qEnvironmentVariableIsSet("D_NO_HARDWARE_ACCELERATION"))
        return false;

    bool envIsIntValue = false;
    bool forceGLPaint = qEnvironmentVariableIntValue("D_USE_GL_PAINT", &envIsIntValue) == 1;
    QVariant value = w->property(enableGLPaint);

    if (envIsIntValue && !forceGLPaint) {
        return false;
    }

    return value.isValid() ? value.toBool() : forceGLPaint;
#else
    return false;
#endif
}

bool DBackingStoreProxy::useWallpaperPaint(const QWindow *w)
{
    return w->property("_d_dxcb_wallpaper").isValid();
}

DBackingStoreProxy::DBackingStoreProxy(QPlatformBackingStore *proxy, bool useGLPaint, bool useWallpaper)
    : QPlatformBackingStore(proxy->window())
    , m_proxy(proxy)
    , enableGL(useGLPaint)
    , enableWallpaper(useWallpaper)
{
#ifndef DISABLE_WALLPAPER
    if (useWallpaper) {
        QObject::connect(DXcbWMSupport::instance(), &DXcbWMSupport::hasWallpaperEffectChanged, window(), &QWindow::requestUpdate);
        QObject::connect(DXcbWMSupport::instance(), &DXcbWMSupport::wallpaperSharedChanged, window(), [ this ] {
            updateWallpaperShared();
        });

        updateWallpaperShared();
    }
#endif
}

DBackingStoreProxy::~DBackingStoreProxy()
{
    delete m_proxy;

    if (m_sharedMemory != nullptr)
        delete m_sharedMemory;
}

QPaintDevice *DBackingStoreProxy::paintDevice()
{
    if (glDevice)
        return glDevice.data();

    return !m_image.isNull()? &m_image : m_proxy->paintDevice();
}

void DBackingStoreProxy::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    if (glDevice)
        return glDevice->flush();

    if (!m_image.isNull()) {
        QRegion expand_region;

        for (const QRect &r : region) {
            expand_region += r.adjusted(-1, -1, 1, 1);
        }

        m_proxy->flush(window, expand_region, offset);
    } else { // 未开启缩放补偿
        m_proxy->flush(window, region, offset);
    }
}

#ifndef QT_NO_OPENGL
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
void DOpenGLBackingStore::composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                                             QPlatformTextureList *textures, QOpenGLContext *context)
{
    m_proxy->composeAndFlush(window, region, offset, textures, context);
}
#elif QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
void DOpenGLBackingStore::composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                                             QPlatformTextureList *textures, QOpenGLContext *context,
                                             bool translucentBackground)
{
    m_proxy->composeAndFlush(window, region, offset, textures, context, translucentBackground);
}
#elif QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
void DBackingStoreProxy::composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                                             QPlatformTextureList *textures,
                                             bool translucentBackground)
{
    m_proxy->composeAndFlush(window, region, offset, textures, translucentBackground);
}
#else
QPlatformBackingStore::FlushResult DBackingStoreProxy::rhiFlush(QWindow *window, qreal sourceDevicePixelRatio, const QRegion &region, const QPoint &offset, QPlatformTextureList *textures, bool translucentBackground)
{
    return m_proxy->rhiFlush(window, sourceDevicePixelRatio, region, offset, textures, translucentBackground);
}
#endif

#if QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
GLuint DBackingStoreProxy::toTexture(const QRegion &dirtyRegion, QSize *textureSize, TextureFlags *flags) const
{
    return m_proxy->toTexture(dirtyRegion, textureSize, flags);
}
#else
QRhiTexture *DBackingStoreProxy::toTexture(QRhiResourceUpdateBatch *resourceUpdates, const QRegion &dirtyRegion, TextureFlags *flags) const
{
    return m_proxy->toTexture(resourceUpdates, dirtyRegion, flags);
}
#endif
#endif

QImage DBackingStoreProxy::toImage() const
{
    return m_proxy->toImage();
}

QPlatformGraphicsBuffer *DBackingStoreProxy::graphicsBuffer() const
{
    return m_proxy->graphicsBuffer();
}

void DBackingStoreProxy::resize(const QSize &size, const QRegion &staticContents)
{
    if (Q_LIKELY(enableGL)) {
        if (Q_LIKELY(glDevice)) {
            glDevice->resize(size);
        } else {
            glDevice.reset(new DOpenGLPaintDevice(window(), DOpenGLPaintDevice::PartialUpdateBlit));
        }

        return;
    }

    m_proxy->resize(size, staticContents);

    if (!QHighDpiScaling::isActive()) {
        // 清理hidpi缓存
        m_image = QImage();
        return;
    }

    qreal scale = QHighDpiScaling::factor(window());
    // 小数倍缩放时才开启此功能
    if (qCeil(scale) != qFloor(scale)) {
        bool hasAlpha = toImage().pixelFormat().alphaUsage() == QPixelFormat::UsesAlpha;
        m_image = QImage(window()->size() * window()->devicePixelRatio(),
                         hasAlpha ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32);
    }
}

bool DBackingStoreProxy::scroll(const QRegion &area, int dx, int dy)
{
    return m_proxy->scroll(area, dx, dy);
}

void DBackingStoreProxy::beginPaint(const QRegion &region)
{
    if (glDevice)
        return;

    m_proxy->beginPaint(region);

    qreal window_scale = window()->devicePixelRatio();

    bool enable = enableWallpaper && !m_wallpaper.isNull();

#ifndef DISABLE_WALLPAPER
    enable = enable && !DXcbWMSupport::instance()->hasWallpaperEffect();
#endif

    if (enable) {
        QPainter p(paintDevice());

        for (QRect rect : region) {
            rect = QHighDpi::fromNativePixels(rect, window());
            rect = QRect(rect.topLeft() * window_scale, QHighDpi::toNative(rect.size(), window_scale));

            // 在指定区域绘制图片
            p.drawImage(rect, m_wallpaper, rect);
            m_dirtyRect |= rect;
        }
        p.end();
    }

    if (m_image.isNull()) {
        return;
    }

    m_dirtyRect = QRect();

    QPainter p(&m_image);
    if (!enable) {
        p.setCompositionMode(QPainter::CompositionMode_Clear);

        for (QRect rect : region) {
            rect = QHighDpi::fromNativePixels(rect, window());
            rect = QRect(rect.topLeft() * window_scale, QHighDpi::toNative(rect.size(), window_scale));

            // 如果是透明绘制，应当先清理要绘制的区域
            if (m_image.format() == QImage::Format_ARGB32_Premultiplied)
                p.fillRect(rect, Qt::transparent);

            m_dirtyRect |= rect;
        }
    }

    p.end();

    if (m_dirtyRect.isValid()) {
        // 额外扩大一个像素，用于补贴两个不同尺寸图片上的绘图误差
        m_dirtyRect.adjust(-window_scale, -window_scale,
                           window_scale, window_scale);

        m_dirtyWindowRect = QRectF(m_dirtyRect.topLeft() / window_scale,
                                   m_dirtyRect.size() / window_scale);
        m_dirtyWindowRect = QHighDpi::toNativePixels(m_dirtyWindowRect, window());
    } else {
        m_dirtyWindowRect = QRectF();
    }
}

void DBackingStoreProxy::endPaint()
{
    if (glDevice)
        return;

    QPainter pa(m_proxy->paintDevice());
    pa.setRenderHints(QPainter::SmoothPixmapTransform);
    pa.setCompositionMode(QPainter::CompositionMode_Source);
    pa.drawImage(m_dirtyWindowRect, m_image, m_dirtyRect);
    pa.end();

    m_proxy->endPaint();
}

void DBackingStoreProxy::updateWallpaperShared()
{
    QString key;
#ifndef DISABLE_WALLPAPER
    key = Utility::windowProperty(window()->winId(),
                                  DXcbWMSupport::instance()->_deepin_wallpaper_shared_key,
                                  XCB_ATOM_STRING,
                                  1024);
#endif
    if (key.isEmpty())
        return;

    if (m_sharedMemory != nullptr) {
        m_wallpaper = QImage();
        delete m_sharedMemory;
        m_sharedMemory = nullptr;
    }

    m_sharedMemory = new QSharedMemory(key);
    if (!m_sharedMemory->attach()) {
        qWarning() << "Unable to attach to shared memory segment.";
        return;
    }

    m_sharedMemory->lock();
    const qint32 *header = reinterpret_cast<const qint32*>(m_sharedMemory->constData());
    const uchar *content = reinterpret_cast<const uchar*>(m_sharedMemory->constData()) + HEADER_SIZE;

    qint32 byte_count = header[0];
    qint32 image_width = header[1];
    qint32 image_height = header[2];
    qint32 image_format = header[3];

    m_wallpaper = QImage(content, image_width, image_height, QImage::Format(image_format));
    m_sharedMemory->unlock();
    window()->requestUpdate();
}

DPP_END_NAMESPACE

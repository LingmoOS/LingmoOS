// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dblitframebuffernode_p.h"

#include <QSurface>
#ifndef QT_NO_OPENGL
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#endif
#include <QQuickItem>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <private/qsgtexture_p.h>
#else
#include <private/qsgplaintexture_p.h>
#endif
#include <private/qquickitem_p.h>

#include <algorithm>

DQUICK_BEGIN_NAMESPACE

class Q_DECL_HIDDEN ProxyAtlasTexture : public QSGPlainTexture {
public:
    bool isAtlasTexture() const override {
        return !subRect.isEmpty();
    }
    QRectF normalizedTextureSubRect() const override {
        if (subRect.isEmpty())
            return QSGPlainTexture::normalizedTextureSubRect();
        return subRect;
    }

    QRectF subRect;
};

#ifndef QT_NO_OPENGL
class Q_DECL_HIDDEN CachedFBO : public QSharedData, public QOpenGLFramebufferObject {
public:
    typedef QPair<QSize, CachedFBO*> CacheData;
    CachedFBO(const QSize &size)
        : QOpenGLFramebufferObject(size)
    {}
    ~CachedFBO() {
        globalFBOList.removeOne(CacheData(size(), this));
    }

    static QExplicitlySharedDataPointer<CachedFBO> getFBO(const QSize &size, bool allowedGreaterSize) {
        auto fboIndex = std::lower_bound(globalFBOList.cbegin(), globalFBOList.cend(), CacheData(size, nullptr),
                                    [] (const CacheData &fbo1, const CacheData &fbo2) {
            const QSize &s1 = fbo1.first;
            const QSize &s2 = fbo2.first;
            return s1.width() < s2.width() || s1.height() < s2.height();
        });

        if (fboIndex != globalFBOList.cend() && !allowedGreaterSize && (*fboIndex).first != size) {
            fboIndex = globalFBOList.cend();
        }

        CachedFBO *fbo = nullptr;
        if (fboIndex == globalFBOList.cend()) {
            fbo = new CachedFBO(size);
            globalFBOList.append(CacheData(size, fbo));
        } else {
            fbo = (*fboIndex).second;
        }

        return QExplicitlySharedDataPointer<CachedFBO>(fbo);
    }

private:
    // order is from small to large by the texture size
    static thread_local QList<CacheData> globalFBOList;
};
thread_local QList<CachedFBO::CacheData> CachedFBO::globalFBOList;
typedef QExplicitlySharedDataPointer<CachedFBO> SharedCachedFBO;

class Q_DECL_HIDDEN GLRenderNode : public DBlitFramebufferNode {
public:
    GLRenderNode(QQuickItem *item)
        : DBlitFramebufferNode(item)
    {

    }

    QImage toImage() const override
    {
        return fbo ? fbo->toImage() : QImage();
    }

    void render(const RenderState *state) override {
        Q_UNUSED(state)
        if (!QOpenGLFramebufferObject::hasOpenGLFramebufferBlit())
            return;

        if (Q_UNLIKELY(!m_item))
            return;

        const qreal scale = m_item->window() ? m_item->window()->effectiveDevicePixelRatio() : 1;
        QRectF sourceRect = matrix()->mapRect(m_rect);
        if (!sourceRect.isValid())
            return;

        sourceRect.moveTopLeft(sourceRect.topLeft() * scale);
        sourceRect.setSize(sourceRect.size() * scale);
        const QSize textureSize = sourceRect.size().toSize();

        if (Q_UNLIKELY(!fbo || (!useAtlasTexture && fbo->size() != textureSize)
                       || (useAtlasTexture && (fbo->width() < textureSize.width()
                                               || fbo->height() < textureSize.height())))) {
            fbo = shareBuffer ? CachedFBO::getFBO(textureSize, useAtlasTexture)
                              : SharedCachedFBO(new CachedFBO(textureSize));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            m_texture->setTextureId(fbo->texture());
            // binding buffer's texture for proxyTexture.
#elif QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
            auto wp = QQuickWindowPrivate::get(m_item->window());
            m_texture->setTextureFromNativeTexture(wp->rhi, static_cast<quint64>(fbo->texture()),
                                                   0, fbo->size(), {}, {});
#else
            auto wp = QQuickWindowPrivate::get(m_item->window());
            m_texture->setTextureFromNativeTexture(wp->rhi, static_cast<quint64>(fbo->texture()),
                                                   0, 0, fbo->size(), {}, {});
#endif
            m_texture->setHasAlphaChannel(true);
            m_texture->setTextureSize(fbo->size());
        }

        if (useAtlasTexture) {
            m_texture->subRect.setSize(QSizeF(sourceRect.width() / fbo->width(),
                                              sourceRect.height() / fbo->height()));
        }
        const QSizeF ss = QOpenGLContext::currentContext()->surface()->size() * scale;
        const QRectF transfromSR(sourceRect.x(), ss.height() - sourceRect.y(),
                                 sourceRect.width(), -sourceRect.height());
        QOpenGLFramebufferObject::blitFramebuffer(fbo.data(), QRect(QPoint(0, 0), textureSize),
                                                  nullptr, transfromSR.toRect());

        doRenderCallback();
    }

private:
    SharedCachedFBO fbo;
};

QSGTexture *DBlitFramebufferNode::texture() const
{
    return m_texture.data();
}

DBlitFramebufferNode *DBlitFramebufferNode::createOpenGLNode(QQuickItem *item,
                                                             bool shareBuffer, bool useAtlasTexture)
{
    auto node = new GLRenderNode(item);
    node->shareBuffer = shareBuffer;
    node->useAtlasTexture = useAtlasTexture;
    return node;
}
#endif

class Q_DECL_HIDDEN CachedQImage : public QSharedData, public QImage {
public:
    typedef QPair<QSize, CachedQImage*> CacheData;
    CachedQImage(const QSize &size)
        : QImage(size, QImage::Format_ARGB32_Premultiplied)
    {}
    ~CachedQImage() {
        globalImageList.removeOne(CacheData(size(), this));
    }

    static QExplicitlySharedDataPointer<CachedQImage> getImage(const QSize &size, bool allowedGreaterSize) {
        auto fboIndex = std::lower_bound(globalImageList.cbegin(), globalImageList.cend(), CacheData(size, nullptr),
                                    [] (const CacheData &image1, const CacheData &image2) {
            const QSize &s1 = image1.first;
            const QSize &s2 = image2.first;
            return s1.width() < s2.width() || s1.height() < s2.height();
        });

        if (fboIndex != globalImageList.cend() && !allowedGreaterSize && (*fboIndex).first != size) {
            fboIndex = globalImageList.cend();
        }

        CachedQImage *image = nullptr;
        if (fboIndex == globalImageList.cend()) {
            image = new CachedQImage(size);
            globalImageList.append(CacheData(size, image));
        } else {
            image = (*fboIndex).second;
        }

        return QExplicitlySharedDataPointer<CachedQImage>(image);
    }

private:
    // order is from small to large by the image size
    static thread_local QList<CacheData> globalImageList;
};
thread_local QList<CachedQImage::CacheData> CachedQImage::globalImageList;
typedef QExplicitlySharedDataPointer<CachedQImage> SharedCachedQImage;

class Q_DECL_HIDDEN SoftwareRenderNode : public DBlitFramebufferNode {
public:
    SoftwareRenderNode(QQuickItem *item)
        : DBlitFramebufferNode(item)
    {

    }

    QImage toImage() const override
    {
        return image ? *image : QImage();
    }

    void render(const RenderState *state) override {
        Q_UNUSED(state)
        if (!m_item->window())
            return;
        QSGRendererInterface *rif = m_item->window()->rendererInterface();
        QPainter *p = static_cast<QPainter *>(rif->getResource(m_item->window(),
                                                               QSGRendererInterface::PainterResource));
        Q_ASSERT(p);

        const auto device = p->device();
        QRectF sourceRect = matrix()->mapRect(m_rect);
        if (!sourceRect.isValid())
            return;

        sourceRect.moveTopLeft(sourceRect.topLeft() * device->devicePixelRatioF());
        sourceRect.setSize(sourceRect.size() * device->devicePixelRatioF());
        const QSize textureSize = sourceRect.size().toSize();

        if (Q_UNLIKELY(!image || (!useAtlasTexture && image->size() != textureSize)
                       || (useAtlasTexture && (image->width() < textureSize.width()
                                               || image->height() < textureSize.height())))) {
            image = shareBuffer ? CachedQImage::getImage(textureSize, useAtlasTexture)
                                : SharedCachedQImage(new CachedQImage(textureSize));
            image->setDevicePixelRatio(device->devicePixelRatioF());
            // ref the image
            QImage textureImage(image->bits(), image->width(), image->height(),
                                image->bytesPerLine(), image->format());
            textureImage.setDevicePixelRatio(image->devicePixelRatio());
            m_texture->setImage(textureImage);
        }

        painter.begin(image.data());
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        if (Q_LIKELY(device->devType() == QInternal::Image)) {
            auto source = static_cast<QImage*>(device);
            painter.drawImage(QPointF(0, 0), *source, sourceRect);
        } else if (device->devType() == QInternal::Pixmap) {
            auto source = static_cast<QPixmap*>(device);
            painter.drawPixmap(QPointF(0, 0), *source, sourceRect);
        }
        painter.end();

        doRenderCallback();
    }

private:
    SharedCachedQImage image;
    QPainter painter;
};

DBlitFramebufferNode *DBlitFramebufferNode::createSoftwareNode(QQuickItem *item,
                                                               bool shareBuffer, bool useAtlasTexture)
{
    auto node = new SoftwareRenderNode(item);
    node->shareBuffer = shareBuffer;
    node->useAtlasTexture = useAtlasTexture;
    return node;
}

QRectF DBlitFramebufferNode::rect() const
{
    return QRectF(0, 0, m_item->width(), m_item->height());
}

void DBlitFramebufferNode::resize(const QSizeF &size)
{
    if (m_size == size)
        return;
    m_size = size;
    m_rect = QRectF(QPointF(0, 0), m_size).marginsAdded(m_margins);
}

void DBlitFramebufferNode::setMargins(const QMarginsF &margin)
{
    if (m_margins == margin)
        return;
    m_margins = margin;
    m_rect = QRectF(QPointF(0, 0), m_size).marginsAdded(m_margins);
}

void DBlitFramebufferNode::setRenderCallback(RenderCallback callback, void *data)
{
    m_renderCallback = callback;
    m_callbackData = data;
}

DBlitFramebufferNode::DBlitFramebufferNode(QQuickItem *item)
    : m_item(item)
    , m_texture(new ProxyAtlasTexture())
{
    m_texture->setOwnsTexture(false);
}

DQUICK_END_NAMESPACE

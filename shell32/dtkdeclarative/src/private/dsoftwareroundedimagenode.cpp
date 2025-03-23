// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsoftwareroundedimagenode_p.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <private/qsgtexture_p.h>
#else
#include <private/qsgplaintexture_p.h>
#endif
#include <private/qquickitem_p.h>
#include <QPainter>

DQUICK_BEGIN_NAMESPACE

// defined in dsoftwareeffectrendernode.cpp
QImage _d_textureConvertToImage(const QSGTexture *texture);

DSoftwareRoundedImageNode::DSoftwareRoundedImageNode(QQuickItem *owner)
    : item(owner)
{

}

void DSoftwareRoundedImageNode::setTexture(QSGTexture *texture)
{
    m_texture = texture;
    markDirty(DirtyMaterial);
}

void DSoftwareRoundedImageNode::setRadius(qreal radius)
{
    if (qFuzzyCompare(radius, this->radius))
        return;
    this->radius = radius;

    clipPath = QPainterPath();
    if (radius > 0) {
        clipPath.addRoundedRect(targetRect, radius, radius);
    }

    markDirty(DirtyMaterial);
}

void DSoftwareRoundedImageNode::setSmooth(bool smooth)
{
    if (this->smooth == smooth)
        return;
    this->smooth = smooth;
    markDirty(DirtyMaterial);
}

void DSoftwareRoundedImageNode::setSourceRect(const QRectF &source)
{
    if (this->sourceRect == source)
        return;
    this->sourceRect = source;
    markDirty(DirtyMaterial);
}

void DSoftwareRoundedImageNode::setRect(const QRectF &target)
{
    if (this->targetRect == target)
        return;
    this->targetRect = target;

    if (radius > 0) {
        clipPath = QPainterPath();
        clipPath.addRoundedRect(targetRect, radius, radius);
    }

    markDirty(DirtyMaterial);
}

void DSoftwareRoundedImageNode::setCompositionMode(QPainter::CompositionMode mode)
{
    if (m_compositionMode == mode)
        return;
    m_compositionMode = mode;
    markDirty(DirtyForceUpdate);
}

void DSoftwareRoundedImageNode::render(const RenderState *state)
{
    if (!sourceRect.isValid())
        return;

    QSGRendererInterface *rif = item->window()->rendererInterface();
    Q_ASSERT(rif->graphicsApi() == QSGRendererInterface::Software);

    QPainter *p = static_cast<QPainter *>(rif->getResource(item->window(),
                                                           QSGRendererInterface::PainterResource));
    Q_ASSERT(p);

    const QRegion *clipRegion = state->clipRegion();
    if (clipRegion && !clipRegion->isEmpty())
        p->setClipRegion(*clipRegion, Qt::ReplaceClip); // must be done before setTransform

    p->setTransform(matrix()->toTransform());
    p->setOpacity(inheritedOpacity());
    p->setRenderHint(QPainter::Antialiasing, smooth);

    if (Q_LIKELY(!clipPath.isEmpty())) {
        p->setClipPath(clipPath, Qt::IntersectClip);
    }

    updateCachedImage();
    p->drawImage(targetRect, cachedImage, convertToTextureNormalizedSourceRect());
}

void DSoftwareRoundedImageNode::releaseResources()
{
    static QImage globalNullImage;
    cachedImage = globalNullImage;
}

QSGRenderNode::RenderingFlags DSoftwareRoundedImageNode::flags() const
{
    RenderingFlags rf = BoundedRectRendering;
    if (Q_UNLIKELY(qIsNull(radius)) && !cachedImage.hasAlphaChannel())
        rf |= OpaqueRendering;
    return rf;
}

QRectF DSoftwareRoundedImageNode::rect() const
{
    return targetRect;
}

void DSoftwareRoundedImageNode::updateCachedImage()
{
    Q_ASSERT(m_texture);
    auto image = _d_textureConvertToImage(m_texture);
    if (!image.isNull()) {
        cachedImage = image;
    } else {
        DSoftwareRoundedImageNode::releaseResources();
    }
}

QRectF DSoftwareRoundedImageNode::convertToTextureNormalizedSourceRect() const
{
    if (!m_texture->isAtlasTexture())
        return sourceRect;
    const QRectF r = m_texture->normalizedTextureSubRect();
    if (r.topLeft().isNull())
        return sourceRect;

    const QSize s = m_texture->textureSize();
    return sourceRect.translated(r.x() * s.width(), r.y() * s.height());
}

DQUICK_END_NAMESPACE

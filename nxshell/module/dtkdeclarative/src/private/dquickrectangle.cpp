// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickrectangle_p.h"
#include "private/dquickrectangle_p_p.h"
#include "private/drectanglenode_p.h"

DQUICK_BEGIN_NAMESPACE

DQuickRectangle::DQuickRectangle(QQuickItem *parent)
    : QQuickItem (*(new DQuickRectanglePrivate), parent)
{
    setFlag(ItemHasContents);
}

DQuickRectangle::~DQuickRectangle()
{
    if (window()) {
        DQuickRectangle::releaseResources();
    }
}

QColor DQuickRectangle::color() const
{
    Q_D(const DQuickRectangle);

    return d->color;
}

void DQuickRectangle::setColor(const QColor &color)
{
    Q_D(DQuickRectangle);

    if (d->color == color)
        return;

    d->color = color;
    update();
    Q_EMIT colorChanged();
}

qreal DQuickRectangle::radius() const
{
    Q_D(const DQuickRectangle);

    return d->radius;
}

void DQuickRectangle::setRadius(qreal radius)
{
    Q_D(DQuickRectangle);

    if(qFuzzyCompare(d->radius, radius))
        return;

    d->radius = radius;
    update();
    Q_EMIT radiusChanged();
}

DQuickRectangle::Corners DQuickRectangle::corners() const
{
    Q_D(const DQuickRectangle);

    return d->corners;
}

void DQuickRectangle::setCorners(Corners corners)
{
    Q_D(DQuickRectangle);

    if (d->corners == corners)
        return;

    d->corners = corners;
    update();
    Q_EMIT cornersChanged();
}

void DQuickRectangle::invalidateSceneGraph()
{
    Q_D(DQuickRectangle);
    d->maskTexture.reset();
}

QSGNode *DQuickRectangle::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    Q_D(DQuickRectangle);

    if (width() <= 0 || height() <= 0 || d->color.alpha() == 0) {
        delete oldNode;
        return nullptr;
    }

    QSGRendererInterface *ri = window()->rendererInterface();
    if (Q_UNLIKELY(ri && ri->graphicsApi() == QSGRendererInterface::Software)) {
        DSoftRectangleNode *rectangle = static_cast<DSoftRectangleNode *>(oldNode);
        if (!rectangle) {
            rectangle = new DSoftRectangleNode(this);
        }

        rectangle->setColor(d->color);
        rectangle->setRadius(d->radius * d->window->effectiveDevicePixelRatio());
        rectangle->setCorners(corners());
        return rectangle;
    } else {
        DRectangleNode *rectangle = static_cast<DRectangleNode *>(oldNode);
        if (!rectangle) {
            rectangle = new DRectangleNode;
        }

        if (d->radius > 0)
            rectangle->setMakTexture(d->textureForRadiusMask());

        rectangle->setRect(QRectF(0, 0, width(), height()));
        rectangle->setColor(d->color);
        rectangle->setRadius(d->radius);
        rectangle->setCorners(corners());
        rectangle->update();

        return rectangle;
    }
}

DQuickRectangle::DQuickRectangle(DQuickRectanglePrivate &dd, QQuickItem *parent)
    : QQuickItem (dd, parent)
{
}

class Q_DECL_HIDDEN DQuickRectangleCleanup : public QRunnable
{
public:
    DQuickRectangleCleanup(MaskTextureCache::TextureData maskTexture)
        : maskTexture(maskTexture)
    {}
    void run() override {
        maskTexture.reset();
    }
    MaskTextureCache::TextureData maskTexture;
};

void DQuickRectangle::releaseResources()
{
    Q_D(DQuickRectangle);
    if (d->maskTexture) {
        window()->scheduleRenderJob(new DQuickRectangleCleanup(d->maskTexture),
                                    QQuickWindow::AfterSynchronizingStage);
        d->maskTexture.reset();
    }
}

DQUICK_END_NAMESPACE

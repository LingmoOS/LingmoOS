// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickglow_p.h"
#include "private/dquickglow_p_p.h"
#include "private/dshadownode_p.h"

DQUICK_BEGIN_NAMESPACE

DQuickGlow::DQuickGlow(QQuickItem *parent)
    : QQuickItem(*(new DQuickGlowPrivate), parent)
{
    setFlag(ItemHasContents, true);
}

qreal DQuickGlow::glowRadius() const
{
    Q_D(const DQuickGlow);

    return d->glowRadius;
}

void DQuickGlow::setGlowRadius(qreal radius)
{
    Q_D(DQuickGlow);

    if(qFuzzyCompare(d->glowRadius, radius))
        return;

    d->glowRadius = radius;
    update();
    Q_EMIT glowRadiusChanged();
}

QColor DQuickGlow::color() const
{
    Q_D(const DQuickGlow);

    return d->color;
}

void DQuickGlow::setColor(const QColor &color)
{
    Q_D(DQuickGlow);

    if (d->color == color)
        return;

    d->color = color;
    update();
    Q_EMIT colorChanged();
}

qreal DQuickGlow::spread() const
{
    Q_D(const DQuickGlow);

    return d->spread;
}

void DQuickGlow::setSpread(qreal radius)
{
    Q_D(DQuickGlow);

    if (qFuzzyCompare(d->spread, radius))
        return;

    d->spread = radius;
    update();
    Q_EMIT spreadChanged();
}

qreal DQuickGlow::relativeSizeX() const
{
    Q_D(const DQuickGlow);

    return d->relativeSizeX;
}

void DQuickGlow::setRelativeSizeX(qreal x)
{
    Q_D(DQuickGlow);

    if (qFuzzyCompare(d->relativeSizeX, x))
        return;

    d->relativeSizeX = x;
    update();
    Q_EMIT relativeSizeXChanged();
}

qreal DQuickGlow::relativeSizeY() const
{
    Q_D(const DQuickGlow);

    return d->relativeSizeY;
}

void DQuickGlow::setRelativeSizeY(qreal y)
{
    Q_D(DQuickGlow);

    if (qFuzzyCompare(d->relativeSizeY, y))
        return;

    d->relativeSizeY = y;
    update();
    Q_EMIT relativeSizeYChangd();
}

bool DQuickGlow::fill()
{
    Q_D(const DQuickGlow);

    return d->fill;
}

void DQuickGlow::setFill(bool isFill)
{
    Q_D(DQuickGlow);

    if (d->fill == isFill)
        return;

    d->fill = isFill;
    update();
    Q_EMIT fillChanged();
}

QSGNode *DQuickGlow::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    Q_D(const DQuickGlow);

    if (width() <= 0 || height() <= 0 || d->color.alpha() == 0) {
        delete oldNode;
        return nullptr;
    }

    ShadowNode *shadow = static_cast<ShadowNode *>(oldNode);
    if (!shadow) {
        QSGRendererInterface *ri = window()->rendererInterface();
        if (Q_UNLIKELY(ri && ri->graphicsApi() == QSGRendererInterface::Software)) {
            // TODO(xiaoyaobing): Software rendering is not supported yet
            return nullptr;
        } else {
            shadow = new ShadowNode;
        }
    }

    shadow->setRect(QRectF(0, 0, width(), height()));
    shadow->setColor(d->color);
    shadow->setGlowRadius(d->glowRadius);
    shadow->setRelativeSizeX(d->relativeSizeX);
    shadow->setRelativeSizeY(d->relativeSizeY);
    shadow->setSpread(d->spread);
    shadow->setFill(d->fill);
    shadow->update();

    return shadow;
}

DQuickGlow::DQuickGlow(DQuickGlowPrivate &dd, QQuickItem *parent)
    : QQuickItem (dd, parent)
{

}

DQUICK_END_NAMESPACE

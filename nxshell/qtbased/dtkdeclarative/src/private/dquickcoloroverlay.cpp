// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcoloroverlaynode.h"
#include "dquickcoloroverlay_p.h"

#include <private/qquickitem_p.h>

DQUICK_BEGIN_NAMESPACE
class DQuickColorOverlayPrivate : public QQuickItemPrivate
{
public:
    DQuickColorOverlayPrivate()
        : color(QColor::Invalid)
        , source(nullptr)
        , sourceChanged(false)
        , cached(true)
    {

    }

    QColor color = Qt::transparent;
    QQuickItem *source;
    bool sourceChanged;
    bool cached;
};

DQuickColorOverlay::DQuickColorOverlay(QQuickItem *parent)
    : QQuickItem (*(new DQuickColorOverlayPrivate), parent)
{
    setFlag(ItemHasContents);
}

DQuickColorOverlay::DQuickColorOverlay(DQuickColorOverlayPrivate &dd, QQuickItem *parent)
    : QQuickItem(dd, parent)
{

}

QColor DQuickColorOverlay::color() const
{
    Q_D(const DQuickColorOverlay);
    return d->color;
}

void DQuickColorOverlay::setColor(const QColor &color)
{
    Q_D(DQuickColorOverlay);
    if (d->color == color)
        return;
    d->color = color;
    Q_EMIT colorChanged();
}

QQuickItem *DQuickColorOverlay::source() const
{
    Q_D(const DQuickColorOverlay);
    return d->source;
}

void DQuickColorOverlay::setSource(QQuickItem *item)
{
    Q_D(DQuickColorOverlay);

    if (item == d->source)
        return;

    d->source = item;
    d->sourceChanged = true;
    Q_EMIT sourceChanged();
    update();
}

void DQuickColorOverlay::setCached(bool cached)
{
    Q_D(DQuickColorOverlay);
    if (d->cached == cached)
        return;
    d->cached = cached;
    Q_EMIT cachedChanged();
    update();
}

bool DQuickColorOverlay::cached() const
{
    Q_D(const DQuickColorOverlay);
    return d->cached;
}

QSGNode *DQuickColorOverlay::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    Q_D(DQuickColorOverlay);
    if (!d->source || !d->source->isTextureProvider()) {
        delete oldNode;
        return nullptr;
    }

    QSGRendererInterface *ri = window()->rendererInterface();
    if (!ri)
        return nullptr;

    QSGRendererInterface::GraphicsApi api = ri->graphicsApi();

    if (QSGRendererInterface::Software == api) {
        DSoftwareColorOverlayNode *node = static_cast<DSoftwareColorOverlayNode *>(oldNode);

        if (d->sourceChanged) {
            delete  node;
            node = nullptr;
            d->sourceChanged = false;
        }

        if (!node)
            node = new DSoftwareColorOverlayNode(d->source->textureProvider());

        node->sync(this);
        node->setCached(d->cached);
        node->setColor(d->color);
        return node;
    }

    return nullptr;
}

DQUICK_END_NAMESPACE

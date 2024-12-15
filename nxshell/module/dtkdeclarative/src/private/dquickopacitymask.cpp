// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickopacitymask_p.h"
#include "dopacitymasknode.h"
#include "dquickopacitymask_p_p.h"

DQUICK_BEGIN_NAMESPACE

DQuickOpacityMask::DQuickOpacityMask(QQuickItem *parent)
    : QQuickItem (*(new DQuickOpacityMaskPrivate), parent)
{
    setFlag(ItemHasContents);
}

bool DQuickOpacityMask::invert()
{
    Q_D(const DQuickOpacityMask);

    return d->invert;
}

void DQuickOpacityMask::setInvert(bool invert)
{
    Q_D(DQuickOpacityMask);

    if (invert == d->invert)
        return;

    d->invert = invert;
    Q_EMIT invertChanged();
    update();
}

QQuickItem *DQuickOpacityMask::source() const
{
    Q_D(const DQuickOpacityMask);

    return d->source;
}

void DQuickOpacityMask::setSource(QQuickItem *item)
{
    Q_D(DQuickOpacityMask);

    if (item == d->source)
        return;

    d->source = item;
    d->sourceChanged = true;
    Q_EMIT sourceChanged();
    update();
}

QQuickItem *DQuickOpacityMask::maskSource() const
{
    Q_D(const DQuickOpacityMask);

    return d->maskSource;
}

void DQuickOpacityMask::setMaskSource(QQuickItem *item)
{
    Q_D(DQuickOpacityMask);

    if (d->maskSource == item)
        return;

    d->maskSource = item;
    d->sourceChanged = true;
    Q_EMIT maskSourceChanged();
    update();
}

QSGNode *DQuickOpacityMask::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    Q_D(DQuickOpacityMask);

    if (!d->source || !d->source->isTextureProvider()
            || !d->maskSource || !d->maskSource->isTextureProvider()) {
        delete oldNode;
        return nullptr;
    }

    QSGRendererInterface *ri = window()->rendererInterface();
    if (!ri)
        return nullptr;

    QSGRendererInterface::GraphicsApi api = ri->graphicsApi();

    if (QSGRendererInterface::Software == api) {
        DSoftwareOpacityMaskNode *node = static_cast<DSoftwareOpacityMaskNode *>(oldNode);

        if (d->sourceChanged) {
            delete  node;
            node = nullptr;
            d->sourceChanged = false;
        }

        if (!node)
            node = new DSoftwareOpacityMaskNode(d->source->textureProvider(), d->maskSource->textureProvider());

        node->sync(this);
        node->setInvert(d->invert);
        return node;
    }

    // NOTE(XiaoYaoBing): OpenGL hardware rendering can be completed by OpacityMask component in QtGraphicalEffects module
    return nullptr;
}

DQuickOpacityMask::DQuickOpacityMask(DQuickOpacityMaskPrivate &dd, QQuickItem *parent)
    : QQuickItem (dd, parent)
{}

DQUICK_END_NAMESPACE

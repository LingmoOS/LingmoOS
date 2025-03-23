// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dopacitymasknode.h"

#include <QQuickWindow>
#include <QQuickItem>
#include <QPainter>
#include <QSGTextureProvider>

DQUICK_BEGIN_NAMESPACE

DSoftwareOpacityMaskNode::DSoftwareOpacityMaskNode(QSGTextureProvider *source, QSGTextureProvider *maskSource)
    : DSoftwareEffectRenderNode(source)
    , m_maskProvider(maskSource)
{
    m_maskImage = updateCachedImage(m_maskProvider);
    if (m_maskProvider) {
        connect(m_maskProvider, &QSGTextureProvider::textureChanged,
                this, &DSoftwareOpacityMaskNode::onMaskTextureChanged, Qt::DirectConnection);
    }
}

void DSoftwareOpacityMaskNode::setInvert(bool invert)
{
    if (invert == m_invert)
        return;

    m_invert = invert;
    markChanged();
}

void DSoftwareOpacityMaskNode::sync(QQuickItem *item)
{
    m_item = item;
    m_width = item->width();
    m_height = item->height();
    return DSoftwareEffectRenderNode::sync(item);
}

QImage DSoftwareOpacityMaskNode::process(const QImage &sourceImage, QPainter::RenderHints hints)
{
    if (Q_UNLIKELY(sourceImage.isNull() || m_maskImage.isNull() || sourceImage.rect() != m_maskImage.rect()))
        return QImage();

    QImage render(sourceImage);
    QPainter painter(&render);
    painter.setRenderHints(hints);
    painter.setPen(Qt::NoPen);
    if (Q_UNLIKELY(!m_invert)) {
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    } else {
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    }

    painter.drawImage(rect(), m_maskImage, m_maskImage.rect());
    painter.end();
    return render;
}

void DSoftwareOpacityMaskNode::preprocess()
{
    if (m_maskProvider && updateTexture(m_maskProvider->texture()))
        markChanged();
    return DSoftwareEffectRenderNode::preprocess();
}

void DSoftwareOpacityMaskNode::onMaskTextureChanged()
{
    QImage maskImage = updateCachedImage(m_maskProvider);
    if (maskImage != m_maskImage) {
        m_maskImage = maskImage;
        markChanged();
    }
}

DQUICK_END_NAMESPACE

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcoloroverlaynode.h"

#include <QPainter>
#include <QQuickItem>

DQUICK_BEGIN_NAMESPACE

DSoftwareColorOverlayNode::DSoftwareColorOverlayNode(QSGTextureProvider *sourceProvider)
    : DSoftwareEffectRenderNode(sourceProvider)
{
}

void DSoftwareColorOverlayNode::setColor(QColor color)
{
    if (color == m_color)
        return;

    m_color = color;
    markChanged();
}

void DSoftwareColorOverlayNode::sync(QQuickItem *item)
{
    m_item = item;
    m_width = item->width();
    m_height = item->height();
    DSoftwareEffectRenderNode::sync(item);
}

QImage DSoftwareColorOverlayNode::process(const QImage &sourceImage, QPainter::RenderHints hints)
{
    QImage device(sourceImage);
    if (Q_UNLIKELY(sourceImage.isNull()))
        return device;

    if (!m_color.isValid() || m_color == Qt::transparent)
        return device;

    QPainter render(&device);
    render.setRenderHints(hints);
    render.setCompositionMode(QPainter::CompositionMode_SourceIn);
    render.fillRect(sourceImage.rect(), m_color);
    return device;
}

DQUICK_END_NAMESPACE

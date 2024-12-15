// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSOFTWARECOLOROVERLAYNODE_H
#define DSOFTWARECOLOROVERLAYNODE_H

#include "dsoftwareeffectrendernode.h"

#include <dtkdeclarative_global.h>

DQUICK_BEGIN_NAMESPACE

class DSoftwareColorOverlayNode : public DSoftwareEffectRenderNode
{
public:
    explicit DSoftwareColorOverlayNode(QSGTextureProvider *sourceProvider);
    void setColor(QColor color);

    void sync(QQuickItem *item) override;
    QRectF rect() const override { return QRect(0, 0, static_cast<int>(m_width), static_cast<int>(m_height)); }
    QImage process(const QImage &sourceImage, QPainter::RenderHints hints) override;

private:
    QColor m_color = Qt::transparent;
    qreal m_width = 0;
    qreal m_height = 0;
    QQuickItem *m_item = nullptr;
};

DQUICK_END_NAMESPACE

#endif  // DSOFTWARECOLOROVERLAYNODE_H

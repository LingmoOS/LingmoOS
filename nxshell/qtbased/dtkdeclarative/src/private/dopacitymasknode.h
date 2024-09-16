// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPACITYMASKNODE_H
#define DOPACITYMASKNODE_H

#include "dsoftwareeffectrendernode.h"

#include <dtkdeclarative_global.h>

DQUICK_BEGIN_NAMESPACE

class DSoftwareOpacityMaskNode : public DSoftwareEffectRenderNode
{
    Q_OBJECT
public:
    explicit DSoftwareOpacityMaskNode(QSGTextureProvider *source, QSGTextureProvider *maskSource);
    void setInvert(bool invert);

    QRectF rect() const override { return QRect(0, 0, static_cast<int>(m_width), static_cast<int>(m_height)); }
    void sync(QQuickItem *item) override;
    QImage process(const QImage &sourceImage, QPainter::RenderHints hints) override;
    void preprocess() override;

private Q_SLOTS:
    void onMaskTextureChanged();

private:
    bool m_invert = false;
    qreal m_width = 0;
    qreal m_height = 0;
    QQuickItem *m_item = nullptr;
    QImage m_maskImage;
    QPointer<QSGTextureProvider> m_maskProvider = nullptr;

};

DQUICK_END_NAMESPACE

#endif // DOPACITYMASKNODE_H

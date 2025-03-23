// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSOFTWAREROUNDEDIMAGENODE_P_H
#define DSOFTWAREROUNDEDIMAGENODE_P_H

#include <dtkdeclarative_global.h>
#include <QSGRenderNode>
#include <QImage>
#include <QPainterPath>
#include <QPainter>

QT_BEGIN_NAMESPACE
class QQuickItem;
class QSGTexture;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class Q_DECL_EXPORT DSoftwareRoundedImageNode : public QSGRenderNode
{
public:
    DSoftwareRoundedImageNode(QQuickItem *owner);

    void setTexture(QSGTexture *texture);
    inline QSGTexture *texture() const
    { return m_texture;}
    void setRadius(qreal radius);
    void setSmooth(bool smooth);
    void setSourceRect(const QRectF &source);
    void setRect(const QRectF &target);
    void setCompositionMode(QPainter::CompositionMode mode);

    void render(const RenderState *state) override;
    void releaseResources() override;
    RenderingFlags flags() const override;
    QRectF rect() const override;

private:
    void updateCachedImage();
    QRectF convertToTextureNormalizedSourceRect() const;

    qreal radius = 0;
    bool smooth = false;
    QRectF sourceRect;
    QRectF targetRect;
    QPainterPath clipPath;
    QPainter::CompositionMode m_compositionMode = QPainter::CompositionMode_SourceOver;

    QSGTexture *m_texture = nullptr;
    QQuickItem *item = nullptr;
    QImage cachedImage;
};

DQUICK_END_NAMESPACE

#endif // DSOFTWAREROUNDEDIMAGENODE_P_H

// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGESVGITEM_H
#define IMAGESVGITEM_H

#include <QGraphicsObject>
#include <QRectF>

#include <DSvgRenderer>

DGUI_USE_NAMESPACE

#include <QtCore/qglobal.h>

#if !defined(QT_NO_GRAPHICSVIEW) && !defined(QT_NO_WIDGETS)

#include <QtWidgets/qgraphicsitem.h>

#include <QtSvg/qtsvgglobal.h>
#include <QGraphicsSvgItem>
class QSvgRenderer;
class ImageSvgItemPrivate;

class LibImageSvgItem : public QGraphicsObject
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(QString elementId READ elementId WRITE setElementId)
    Q_PROPERTY(QSize maximumCacheSize READ maximumCacheSize WRITE setMaximumCacheSize)

public:
    LibImageSvgItem(QGraphicsItem *parentItem = nullptr);
    LibImageSvgItem(const QString &fileName, QGraphicsItem *parentItem = nullptr);
    ~LibImageSvgItem() override;

    void setSharedRenderer(QSvgRenderer *renderer);
    QSvgRenderer *renderer() const;

    void setElementId(const QString &id);
    QString elementId() const;

    void setCachingEnabled(bool);
    bool isCachingEnabled() const;

    void setMaximumCacheSize(const QSize &size);
    QSize maximumCacheSize() const;

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    enum { Type = 13 };
    int type() const override;

    void updateDefaultSize();
private:

    QSvgRenderer *m_renderer = nullptr;
    QRectF m_boundingRect;
    QString m_elemId;

//    Q_PRIVATE_SLOT(d_func(), void _q_repaintItem())
};

#endif // QT_NO_GRAPHICSVIEW or QT_NO_WIDGETS

#endif // IMAGESVGITEM_H

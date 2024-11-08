/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kbubblewidget.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QRegion>
#include <KWindowEffects>
#include "themeController.h"
#include <QPainterPath>

namespace kdk {

class KBubbleWidgetPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KBubbleWidget)

public:
    KBubbleWidgetPrivate(KBubbleWidget*parent);

private:
    KBubbleWidget *q_ptr;

    int m_startX;
    int m_startY;
    int m_tailWidth;
    int m_tailHeight;
    TailDirection m_tailDirection;
    TailLocation m_tailLocation;
    int m_topLeftRadius;
    int m_topRightRadius;
    int m_bottomLeftRadius;
    int m_bottomRightRadius;
    bool m_enableBlur;
    qreal m_opacity;

};

void KBubbleWidget::setTailPosition(TailDirection dirType, TailLocation locType)
{
    Q_D(KBubbleWidget);
    d->m_tailDirection = dirType;
    d->m_tailLocation  = locType;
    this->update();
}

TailDirection KBubbleWidget ::tailDirection()
{
    Q_D(KBubbleWidget);
    return d->m_tailDirection;
}

TailLocation KBubbleWidget::tailLocation()
{
    Q_D(KBubbleWidget);
    return d->m_tailLocation;
}

void KBubbleWidget::setBorderRadius(int bottomLeft,int topLeft,int topRight,int bottomRight)
{
    Q_D(KBubbleWidget);
    d->m_bottomRightRadius = bottomRight;
    d->m_topLeftRadius = topLeft;
    d->m_bottomLeftRadius = bottomLeft;
    d->m_topRightRadius = topRight;
}

void KBubbleWidget::setBorderRadius(int radius)
{
    Q_D(KBubbleWidget);
    d->m_topLeftRadius = radius;
    d->m_bottomLeftRadius = radius;
    d->m_bottomRightRadius = radius;
    d->m_topRightRadius = radius;
}

void KBubbleWidget::setEnableBlur(bool flag)
{
    Q_D(KBubbleWidget);
    d->m_enableBlur = flag;
}

bool KBubbleWidget::enableBlur()
{
    Q_D(KBubbleWidget);
    return d->m_enableBlur;
}

void KBubbleWidget::setOpacity(qreal opacity)
{
    Q_D(KBubbleWidget);
    if(opacity < 0 || opacity > 1)
        return;
    d->m_opacity = opacity;
}

qreal KBubbleWidget::opacity()
{
    Q_D(KBubbleWidget);
    return d->m_opacity;
}

KBubbleWidget::KBubbleWidget(QWidget *parent)
    :QWidget(parent),
      d_ptr(new KBubbleWidgetPrivate(this))
{
    Q_D(KBubbleWidget);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void KBubbleWidget::setTailSize(const QSize &size)
{
    Q_D(KBubbleWidget);
    d->m_tailWidth = size.width();
    d->m_tailHeight = size.height();
}

QSize KBubbleWidget::tailSize()
{
    Q_D(KBubbleWidget);
    QSize size;
    size.setWidth(d->m_tailWidth);
    size.setHeight(d->m_tailHeight);
    return size;
}

void KBubbleWidget ::paintEvent(QPaintEvent *)
{
    Q_D(KBubbleWidget);

    QPolygon trianglePolygon;
    QPainterPath path;
    QRect targetRect = this->rect();
    switch (d->m_tailDirection)
    {
    case TopDirection:
        targetRect = targetRect.adjusted(0,d->m_tailHeight,0,0);

        if(d->m_tailLocation == LeftLocation)
            d->m_startX = 18;
        else if(d->m_tailLocation == MiddleLocation)
            d->m_startX = (targetRect.width()-d->m_tailWidth*2)/2;
        else if(d->m_tailLocation == RightLocation)
            d->m_startX = targetRect.width()- d->m_tailHeight*2-d->m_tailWidth-19;
        d->m_startY = 0;
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight, d->m_tailHeight);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth / 2 + d->m_tailHeight, 0);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth + d->m_tailHeight, d->m_tailHeight);
        break;

    case LeftDirection:
        targetRect = targetRect.adjusted(d->m_tailHeight,0,0,0);

        if(d->m_tailLocation == LeftLocation)
            d->m_startY = d->m_tailHeight +18;
        else if(d->m_tailLocation == MiddleLocation)
            d->m_startY = targetRect.height()/2-d->m_tailWidth/2;
        else if(d->m_tailLocation == RightLocation)
             d->m_startY = d->m_tailHeight + static_cast<int>((targetRect.height()-d->m_tailHeight*2 - d->m_tailWidth))-19;
        d->m_startX = 0;
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight, d->m_startY);
        trianglePolygon << QPoint(d->m_startX , d->m_startY + d->m_tailWidth/2);
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight , d->m_startY + d->m_tailWidth);
        break;

    case RightDirection:
        targetRect = targetRect.adjusted(0,0,-d->m_tailHeight,0);

        if(d->m_tailLocation == LeftLocation)
            d->m_startY = d->m_tailHeight +18;
        else if(d->m_tailLocation == MiddleLocation)
            d->m_startY = targetRect.height()/2-d->m_tailWidth/2;
        else if(d->m_tailLocation == RightLocation)
             d->m_startY = d->m_tailHeight + static_cast<int>((targetRect.height()-d->m_tailHeight*2 - d->m_tailWidth))-19;
        d->m_startX = targetRect.width();
        trianglePolygon << QPoint(d->m_startX - d->m_tailHeight - 1, d->m_startY);
        trianglePolygon << QPoint(d->m_startX - 1, d->m_startY + d->m_tailWidth/2);
        trianglePolygon << QPoint(d->m_startX - d->m_tailHeight - 1 , d->m_startY + d->m_tailWidth);

        break;
    case BottomDirection:
        targetRect = targetRect.adjusted(0,0,0,-d->m_tailHeight);

        if(d->m_tailLocation == LeftLocation)
            d->m_startX = 18;
        else if(d->m_tailLocation == MiddleLocation)
            d->m_startX = (this->rect().width()-d->m_tailWidth*2)/2;
        else if(d->m_tailLocation == RightLocation)
            d->m_startX = this->rect().width()- d->m_tailHeight*2-d->m_tailWidth-19;
        d->m_startY = this->rect().height();
        trianglePolygon << QPoint(d->m_startX + d->m_tailHeight, d->m_startY - d->m_tailHeight-1);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth / 2 + d->m_tailHeight, d->m_startY-1);
        trianglePolygon << QPoint(d->m_startX + d->m_tailWidth + d->m_tailHeight,d->m_startY - d->m_tailHeight-1);

        break;
    case None:
        break;
    }

    if(ThemeController::widgetTheme() == ClassicTheme)
    {
        path.addRect(targetRect.adjusted(0,0,-1,-1));
    }
    else
    {
        path.moveTo(targetRect.topRight() - QPoint(d->m_topRightRadius , 0) ); //右上
        path.lineTo(targetRect.topLeft() + QPointF(d->m_topLeftRadius , 0)); //上方线
        path.quadTo(targetRect.topLeft(), targetRect.topLeft() + QPointF(0 , d->m_topLeftRadius)); //圆角
        path.lineTo(targetRect.bottomLeft() +  QPointF(0,- d->m_bottomLeftRadius)); //左方线
        path.quadTo(targetRect.bottomLeft(), targetRect.bottomLeft() + QPointF(d->m_bottomLeftRadius , 0));//圆角
        path.lineTo(targetRect.bottomRight() - QPointF(d->m_bottomRightRadius , 0));//下方线
        path.quadTo(targetRect.bottomRight(), targetRect.bottomRight() + QPointF(0, - d->m_bottomRightRadius));//圆角
        path.lineTo(targetRect.topRight() + QPointF(0, d->m_topRightRadius));//右方线
        path.quadTo(targetRect.topRight() , targetRect.topRight() - QPointF(d->m_topRightRadius, 0)); // 圆角
    }

    path.addPolygon(trianglePolygon);
    path = path.simplified();//合并subpath,取消交集部分的线

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    if(d->m_enableBlur)
    {
        //开启毛玻璃时
        QRegion region(path.toFillPolygon().toPolygon());
        KWindowEffects::enableBlurBehind(this->winId(),true,region);
        this->setMask(rect());
        painter.setOpacity(d->m_opacity);
    }
    else
    {
        //未开启毛玻璃时
        painter.setOpacity(1);
    }
    QPen pen;
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setWidthF(1);
    pen.setColor(Qt::gray);
    painter.translate(0.5,0.5);
    painter.setPen(pen);
    painter.setBrush(this->palette().color(this->backgroundRole()));
    painter.drawPath(path);
}

KBubbleWidgetPrivate::KBubbleWidgetPrivate(KBubbleWidget *parent)
    :q_ptr(parent),
      m_bottomLeftRadius(8),
      m_topLeftRadius(8),
      m_bottomRightRadius(8),
      m_topRightRadius(8),
      m_tailHeight(8),
      m_tailWidth(16),
      m_enableBlur(false),
      m_opacity(0.5),
      m_tailLocation(TailLocation::LeftLocation),
      m_tailDirection(TailDirection::BottomDirection)
{
}
}
#include "kbubblewidget.moc"
#include "moc_kbubblewidget.cpp"


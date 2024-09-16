// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gridline.h"
#include "videowidget.h"

#include <QPainter>
#include <QGraphicsScene>

const QColor gridLineColor = QColor(255, 255, 255, 0.5 * 255);// 网格线颜色值

GridLineItem::GridLineItem(QGraphicsItem *parent) : QGraphicsItem (parent)
{

}

void GridLineItem::setGridType(GridType type)
{
    if (type == m_gridType)
        return;

    m_gridType = type;
    update();
}

QRectF GridLineItem::boundingRect() const
{
    return m_boundingRect;
}

void GridLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 画布大小-1，保证网格线显示在画面内
    m_boundingRect = scene()->sceneRect().adjusted(0, 0, -1, -1);
  
    // 显示未连接摄像图标时，画布大小不足140，为满足网格线拉满窗口显示需求，需要重新计算网格线的位置和大小
    if (m_boundingRect.width() <= 140) {
        QRectF rtWnd = widget->topLevelWidget()->rect();
        rtWnd.moveTo(-rtWnd.width() / 2 + m_boundingRect.width() / 2, -rtWnd.height() / 2 + m_boundingRect.height() / 2);
        m_boundingRect = rtWnd;
    }

    if (Grid_None == m_gridType)
        return;

    QRect rtGrid = m_boundingRect.toRect();

    // 绘制网格线
    QPen pen;
    pen.setColor(QColor(gridLineColor));
    pen.setWidth(1);
    painter->setPen(pen);
    painter->save();
    qreal fHSpace = static_cast<qreal>(rtGrid.width()) / (m_gridType + 1);
    qreal fVSpace = static_cast<qreal>(rtGrid.height()) / (m_gridType + 1);
    for (int i = 0; i < m_gridType; i++) {
        QLineF HLine(QPointF(rtGrid.left(), rtGrid.top() + fVSpace * (i + 1)), QPointF(rtGrid.right(), rtGrid.top() + fVSpace * (i + 1)));
        QLineF VLine(QPointF(rtGrid.left() + fHSpace * (i + 1), rtGrid.top()), QPointF(rtGrid.left() + fHSpace * (i + 1), rtGrid.bottom()));
        painter->drawLine(HLine);
        painter->drawLine(VLine);
    }
    painter->restore();
}

GridLineWidget::GridLineWidget(QWidget *parent) : QWidget(parent)
{
    m_videowidget = qobject_cast<videowidget*>(parent);
}

void GridLineWidget::setGridType(GridType type)
{
    m_gridType = type;
    update();
}

void GridLineWidget::paintEvent(QPaintEvent *e)
{
    QRect rtGrid = rect();

    //if the video frame area is zero, it will not be repainted,
    //otherwize it will be repainted outside the video frame.
    if (m_videowidget->getFrameRect().isNull())
        return;

    if (m_videowidget)
        // 画面大小-1，是为了保证网格线等分距离与网格线图元等分距离相等，这样能保证两种网格线能重合显示，避免视觉上网格线微移情况
        rtGrid = m_videowidget->getFrameRect().adjusted(0, 0, -1, -1);

    QPainter painter(this);
    QPen pen;
    pen.setColor(gridLineColor);
    pen.setWidth(1);
    painter.setPen(Qt::NoPen);
    painter.save();
    painter.fillRect(rtGrid, Qt::transparent);
    painter.restore();
    painter.setPen(pen);

    if (Grid_None == m_gridType)
        return;

    // 绘制网格线
    painter.save();
    qreal fHSpace = static_cast<qreal>(rtGrid.width()) / (m_gridType + 1);
    qreal fVSpace = static_cast<qreal>(rtGrid.height()) / (m_gridType + 1);
    rtGrid.adjust(0, 0, 1, 1);//画面大小恢复到原始大小，保证网格线显示填满画面边缘
    for (int i = 0; i < m_gridType; i++) {
        QLineF HLine(QPointF(rtGrid.left(), rtGrid.top() + fVSpace * (i + 1)), QPointF(rtGrid.right(), rtGrid.top() + fVSpace * (i + 1)));
        QLineF VLine(QPointF(rtGrid.left() + fHSpace * (i + 1), rtGrid.top()), QPointF(rtGrid.left() + fHSpace * (i + 1), rtGrid.bottom()));
        painter.drawLine(HLine);
        painter.drawLine(VLine);
    }
    painter.restore();
}

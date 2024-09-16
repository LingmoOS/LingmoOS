// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "chameleonwidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QPainterPath>

ChameleonWidget::ChameleonWidget(QWidget *parent)
    : MouseStateWidget(parent)
    , m_beChameleon(true)
    , m_dragging(false)
    , m_active(false)
{

}

ChameleonWidget::~ChameleonWidget()
{

}

void ChameleonWidget::setDraggingState(bool dragging)
{
    if (m_dragging != dragging) {
        m_dragging = dragging;
        update();
    }
}

void ChameleonWidget::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        update();
    }
}

void ChameleonWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (!m_beChameleon || (!m_state && !m_active) || m_dragging)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int radius = 6;
    QColor backColor = Qt::white;
    if (m_state & HOVERD) {
        backColor.setAlphaF(m_active ? 0.25 : 0.15);
    } else if (m_state & PRESSED) {
        backColor.setAlphaF(m_active ? 0.30 : 0.25);
    } else {
        backColor.setAlphaF(0.15);
    }
    QRectF rc = this->rect();
    const int borderWidth = 1;
    QPen pen;

    // 背景透明
    painter.fillRect(rc, Qt::transparent);

     // 不开缩放用常规方法
    if (qFuzzyCompare(devicePixelRatioF(), 1.0)) {
        // 绘制外边框线
        pen.setColor(QColor(0, 0, 0, 0.10 * 255));
        pen.setWidth(borderWidth);
        painter.setPen(pen);
        painter.drawRoundedRect(rc, radius, radius);

        // 绘制背景色
        const int backRadius = (radius - borderWidth) > 0 ? radius - borderWidth : 0;
        rc = rc.adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
        QPainterPath path;
        path.addRoundedRect(rc, backRadius, backRadius);
        painter.fillPath(path, backColor);

        // 绘制内边框线
        pen.setColor(QColor(255, 255, 255, 0.10 * 255));
        pen.setWidth(borderWidth);
        painter.setPen(pen);
        painter.drawRoundedRect(rc, backRadius, backRadius);
    } else {
        const qreal devicePixelRatio = qFuzzyCompare(devicePixelRatioF(), 0.0) ? 1.0 : devicePixelRatioF();
        const qreal borderWidthOffset = borderWidth / devicePixelRatio;

        // 空1px处理外边框线
        rc = rc.marginsRemoved(QMargins(borderWidth, borderWidth, borderWidth, borderWidth));

        // 绘制外边框线
        pen.setColor(QColor(0, 0, 0, 0.10 * 255));
        pen.setWidthF(borderWidthOffset);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(pen);
        drawBorder(&painter, rc, radius);

        // 绘制背景色
        const int backRadius = (radius - borderWidth) > 0 ? radius - borderWidth : 0;
        rc = rc.adjusted(borderWidthOffset, borderWidthOffset, -borderWidthOffset, -borderWidthOffset);
        painter.setPen(Qt::NoPen);
        QPainterPath path;
        path.addRoundedRect(rc, backRadius, backRadius);
        painter.fillPath(path, backColor);

        // 绘制内边框线
        pen.setColor(QColor(255, 255, 255, 0.10 * 255));
        pen.setWidthF(borderWidthOffset);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(pen);
        drawBorder(&painter, rc, backRadius);
    }
}

void ChameleonWidget::drawBorder(QPainter *p, const QRectF &rect, double radius)
{
    p->setRenderHint(QPainter::RenderHint::Antialiasing, false);

    // 绘制直边
    p->drawLine(rect.topLeft() + QPointF(radius, 0), rect.topRight() - QPointF(radius, 0));
    p->drawLine(rect.bottomLeft() + QPointF(radius, 0), rect.bottomRight() - QPointF(radius, 0));
    p->drawLine(rect.topLeft() + QPointF(0, radius), rect.bottomLeft() - QPointF(0, radius));
    p->drawLine(rect.topRight() + QPointF(0, radius), rect.bottomRight() - QPointF(0, radius));

    QPen pen = p->pen();
    pen.setCapStyle(Qt::FlatCap); // 设置线条端点样式为平直端点
    p->setPen(pen);
    p->setRenderHint(QPainter::RenderHint::Antialiasing, true); // 圆角需要开启抗锯齿

    // 绘制圆角 - 左上角
    QRectF leftTopRect(rect.topLeft().x(), rect.topLeft().y(), radius * 2, radius * 2);
    p->drawArc(leftTopRect, 90 * 16, 90 * 16);
    // 右上角
    QRectF rightTopRect(rect.topRight().x() - radius * 2, rect.topRight().y(), radius * 2, radius * 2);
    p->drawArc(rightTopRect, 0, 90 * 16);
    // 左下角
    QRectF leftBottomRect(rect.bottomLeft().x(), rect.bottomLeft().y() - radius * 2, radius * 2, radius * 2);
    p->drawArc(leftBottomRect, 180 * 16, 90 * 16);
    // 右下角
    QRectF rightBottomRect(rect.bottomRight().x() - radius * 2, rect.bottomRight().y() - radius * 2, radius * 2, radius * 2);
    p->drawArc(rightBottomRect, 270 * 16, 90 * 16);
}


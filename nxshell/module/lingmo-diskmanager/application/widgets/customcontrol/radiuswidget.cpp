// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "radiuswidget.h"

RadiusWidget::RadiusWidget(QWidget *parent) : QWidget(parent)
{
    m_leftTop = false;
    m_leftBottom = false;
    m_rightTop = false;
    m_rightBottom = false;
    m_isEnter = false;
    m_isChecked = false;
}

void RadiusWidget::setMode(bool leftTop, bool leftBottom, bool rightTop, bool rightBottom)
{
    m_leftTop = leftTop;
    m_leftBottom = leftBottom;
    m_rightTop = rightTop;
    m_rightBottom = rightBottom;
}

void RadiusWidget::setChecked(bool isChecked)
{
    m_isChecked = isChecked;
    update();
}

void RadiusWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        if (m_isChecked) {
            painter.setBrush(QBrush(QColor(0, 0, 0, 30)));
        } else {
            if (m_isEnter) {
                painter.setBrush(QBrush(QColor(0, 0, 0, 30)));
            } else {
                painter.setBrush(QBrush(QColor(0, 0, 0, 10)));
            }
        }
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        if (m_isChecked) {
            painter.setBrush(QBrush(QColor(255, 255, 255, 30)));
        } else {
            if (m_isEnter) {
                painter.setBrush(QBrush(QColor(255, 255, 255, 30)));
            } else {
                painter.setBrush(QBrush(QColor(255, 255, 255, 10)));
            }
        }
    }

    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width());
    rect.setHeight(rect.height());

    QPointF pointstop[4] = {
        QPointF(rect.topLeft().x() + 10, rect.topLeft().y()),
        QPointF(rect.topLeft().x() + 10, rect.topLeft().y() + 10),
        QPointF(rect.topRight().x() - 10, rect.topLeft().y() + 10),
        QPointF(rect.topRight().x() - 10, rect.topLeft().y())
    };
    painter.drawPolygon(pointstop, 4);

    QPointF pointscenter[4] = {
        QPointF(rect.topLeft().x(), rect.topLeft().y() + 10),
        QPointF(rect.bottomLeft().x(), rect.bottomLeft().y() - 10),
        QPointF(rect.bottomRight().x(), rect.bottomRight().y() - 10),
        QPointF(rect.topRight().x(), rect.topRight().y() + 10)

    };
    painter.drawPolygon(pointscenter, 4);

    QPointF pointsbottom[4] = {
        QPointF(rect.bottomLeft().x() + 10, rect.bottomLeft().y() - 10),
        QPointF(rect.bottomLeft().x() + 10, rect.bottomLeft().y()),
        QPointF(rect.bottomRight().x() - 10, rect.bottomRight().y()),
        QPointF(rect.bottomRight().x() - 10, rect.bottomRight().y() - 10)
    };
    painter.drawPolygon(pointsbottom, 4);

    //给各个角画直角
    if (m_leftTop) {
        QRectF rectf(QPointF(rect.topLeft().x(), rect.topLeft().y()), QSizeF(20, 20));
        painter.drawPie(rectf, 90 * 16, 90 * 16);
    } else {
        painter.drawPolygon(QRect(QPoint(rect.topLeft()), QSize(10, 10)));
    }

    if (m_leftBottom) {
        QRectF rectf(QPointF(rect.bottomLeft().x(), rect.bottomLeft().y() - 20), QSizeF(20, 20));
        painter.drawPie(rectf, 180 * 16, 90 * 16);
    } else {
        painter.drawPolygon(QRect(QPoint(rect.bottomLeft().x(), rect.bottomLeft().y() - 10), QSize(10, 10)));
    }

    if (m_rightTop) {
        QRectF rectf(QPointF(rect.topRight().x() - 20, rect.topRight().y()), QSizeF(20, 20));
        painter.drawPie(rectf, 0 * 16, 90 * 16);
    } else {
        painter.drawPolygon(QRect(QPoint(rect.topRight().x() - 10, rect.topRight().y()), QSize(10, 10)));
    }

    if (m_rightBottom) {
        QRectF rectf(QPointF(rect.bottomRight().x() - 20, rect.bottomRight().y() - 20), QSizeF(20, 20));
        painter.drawPie(rectf, 270 * 16, 90 * 16);
    } else {
        painter.drawPolygon(QRect(QPoint(rect.bottomRight().x() - 10, rect.bottomRight().y() - 10), QSize(10, 10)));
    }

    QWidget::paintEvent(event);
}

void RadiusWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_isEnter =true;
    update();
}

void RadiusWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_isEnter = false;
    update();
}



/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "fontsize_color_chose.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QVector4D>
#define SHADOW_WIDTH  5     // 小三角的阴影宽度
#define TRIANGLE_WIDTH 20    // 小三角的宽度
#define TRIANGLE_HEIGHT 10   // 小三角的高度
//#define BORDER_RADIUS 6       // 窗口边角弧度

FontSize_Color_Chose::FontSize_Color_Chose(QWidget *parent) :
    QWidget(parent),
    m_startx(5),
    m_triangleWidth(TRIANGLE_WIDTH),
    m_triangleHeight(TRIANGLE_HEIGHT),
    radius(1.5)
{
    context.style_settings = new QGSettings("org.lingmo.style");
    context.style_name = context.style_settings->get("style-name").toString();
    setCursor(Qt::ArrowCursor);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocus();
    // 设置阴影边框
// auto shadowEffect = new QGraphicsDropShadowEffect(this);
// shadowEffect->setOffset(0,0);
// shadowEffect->setColor(Qt::gray);
// shadowEffect->setBlurRadius(BORDER_RADIUS);
// this->setGraphicsEffect(shadowEffect);
    setProperty("useCustomShadow", true); // 启用协议
    setProperty("customShadowDarkness", 1.0); // 阴影暗度
    setProperty("customShadowWidth", 30); // 阴影边距大小
    setProperty("customShadowRadius", QVector4D(1, 1, 1, 1)); // 阴影圆角，必须大于0，这个值应该和frameless窗口本身绘制的形状吻合
    setProperty("customShadowMargins", QVector4D(30, 30, 30, 30)); // 阴影与窗口边缘的距离，一般和customShadowWidth保持一致
    this->setMouseTracking(true);
    Start_x = 15;
    Start_y = 39;
    for (int i = 0; i < 4; i++) {
        m_colorAreaList.append(QRect(Start_x, Start_y, radius*2, radius*2));
        Start_x += radius*2;
        Start_x += 10;
        Start_y -= (i+1);
        radius += (i+1);
    }
    Start_x = 96;
    Start_y = 22;
    for (int i = 4; i < 8; i++) {
        m_colorAreaList.append(QRect(Start_x, Start_y, 14, 14));
        Start_x += 24;
    }
    Start_x = 96;
    Start_y = 45;
    for (int i = 8; i < 12; i++) {
        m_colorAreaList.append(QRect(Start_x, Start_y, 14, 14));
        Start_x += 24;
    }
    m_colorAreaList.append(QRect(5, 15, 16, 46));
    m_colorAreaList.append(QRect(22, 15, 13, 46));
    m_colorAreaList.append(QRect(36, 15, 17, 46));
    m_colorAreaList.append(QRect(54, 15, 30, 46));
}

void FontSize_Color_Chose::setStartPos(double startX)
{
    m_startx = startX;
}

void FontSize_Color_Chose::setTriangleInfo(double width, double height)
{
    m_triangleWidth = 20;
    m_triangleHeight = 10;
}

void FontSize_Color_Chose::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    //
    QPolygon trianglePolygon;
    trianglePolygon << QPoint(m_startx, m_triangleHeight+ SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startx+m_triangleWidth/2, SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startx + m_triangleWidth, m_triangleHeight + SHADOW_WIDTH);
    QPainterPath drawPath;
    int borderRadius =  "classical" == context.style_settings->get("widgetThemeName").toString() ? 0 : 6;
    drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
                                  width()- SHADOW_WIDTH, height()- m_triangleHeight-SHADOW_WIDTH),
                            borderRadius, borderRadius, Qt::AbsoluteSize);
    drawPath.addPolygon(trianglePolygon);
    if ((context.style_name.compare("lingmo-dark") == 0)
        || (context.style_name.compare("lingmo-black") == 0)) {
        painter.setBrush(QColor(25, 25, 25, 180));
        painter.drawPath(drawPath);
        for (int i = 12; i < 16; i++) {
            QRect rect = m_colorAreaList.at(i);
            painter.setBrush(QColor(Qt::black));
            painter.setOpacity(0);
            painter.drawEllipse(rect);
        }
        painter.setOpacity(1);
    } else {
        // if((context.style_name.compare("lingmo-white")==0) || (context.style_name.compare("lingmo-default")==0) || (context.style_name.compare("lingmo-light")==0)){
        painter.setBrush(QColor(225, 225, 225, 180));
        painter.drawPath(drawPath);
        for (int i = 12; i < 16; i++) {
            QRect rect = m_colorAreaList.at(i);
            painter.setBrush(QColor(Qt::gray));
            painter.setOpacity(0);
            painter.drawEllipse(rect);
        }
        painter.setOpacity(1);
    }
    for (int i = 0; i < 4; i++) {
        QRect rect = m_colorAreaList.at(i);
        if (color_rect == rect) {
            Rect_h = rect.height()+4;
            Rect_w = rect.width()+4;
            painter.setBrush(QColor(255, 255, 255));
            painter.drawEllipse(QRect(rect.x()-2, rect.y()-2, Rect_h, Rect_w));
            if ((context.style_name.compare("lingmo-dark") == 0)
                || (context.style_name.compare("lingmo-black") == 0)) {
                painter.setBrush(QColor(Qt::gray));
            } else
                painter.setBrush(QColor(0, 0, 0));
            painter.drawEllipse(rect);
        } else {
            if ((context.style_name.compare("lingmo-dark") == 0)
                || (context.style_name.compare("lingmo-black") == 0)) {
                painter.setBrush(QColor(Qt::gray));
            } else
                painter.setBrush(QColor(78, 78, 78));
            painter.drawEllipse(rect);
        }
    }
    if ((context.style_name.compare("lingmo-dark") == 0)
        || (context.style_name.compare("lingmo-black") == 0)) {
        painter.setBrush(QColor(Qt::gray));
    } else
        painter.setBrush(QColor(25, 25, 25, 153));
    painter.drawRect(85, 33, 1.52, 16);
    for (int i = 4; i < 12; i++) {
        QRect rect = m_colorAreaList.at(i);
        QColor color1 = m_colorList.at(i-4);
        if (color == color1) {
            Rect_h = rect.height()+4;
            Rect_w = rect.width() +4;
            painter.setBrush(QColor(255, 255, 255));
            painter.drawRoundedRect(QRect(rect.x()-2,
                                          rect.y()-2, Rect_h, Rect_w), 2, 2, Qt::AbsoluteSize);
        }
        painter.setBrush(color1);
        painter.drawRoundedRect(m_colorAreaList.at(i), 2, 2, Qt::AbsoluteSize);
    }
}

void FontSize_Color_Chose::mousePressEvent(QMouseEvent *e)
{
    for (int i = 0; i < 12; i++) {
        switch (i) {
        case 0:
            if (m_colorAreaList.at(12).contains(e->pos())) {
                color_rect = m_colorAreaList.at(i);
                emit font_size_change(1);
            }
            break;
        case 1:
            if (m_colorAreaList.at(13).contains(e->pos())) {
                color_rect = m_colorAreaList.at(i);
                emit font_size_change(3);
            }
            break;
        case 2:
            if (m_colorAreaList.at(14).contains(e->pos())) {
                color_rect = m_colorAreaList.at(i);
                emit font_size_change(5);
            }
            break;
        case 3:
            if (m_colorAreaList.at(15).contains(e->pos())) {
                color_rect = m_colorAreaList.at(i);
                emit font_size_change(8);
            }
            break;
        default:
            if (m_colorAreaList.at(i).contains(e->pos())) {
                color = m_colorList.at(i-4);
                if (i == 11) {
                    emit colorSelected(QColor(225, 225, 225));
                } else
                    emit colorSelected(color);
                break;
            }
            update();
        }
    }
}

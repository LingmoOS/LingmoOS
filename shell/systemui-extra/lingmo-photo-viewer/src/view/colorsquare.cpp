/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "colorsquare.h"

ColorSquare::ColorSquare(QWidget *parent) :
    QFrame(parent)
  , huem(0)
  , sat(0)
  , val(0)
  , colorX(0)
  , colorY(0)
  , colorChar('0')
  , mouseStatus(Nothing)
{
    setCursor(Qt::CrossCursor);
    setFixedSize(266,200);
}

QColor ColorSquare::color() const
{
    return QColor::fromHsvF(huem,sat,val);
}

void ColorSquare::setColor(QColor c)
{
    // 色调
    huem = c.hueF();
    if ( huem < 0 )
        huem = 0;
    // 饱和度
    sat = c.saturationF();
    // 色值
    val = c.valueF();

    update();
    Q_EMIT colorChanged(c);
}

void ColorSquare::setHue(qreal h)
{
    huem = qMax(0.0,qMin(1.0,h));
    update();
}

void ColorSquare::setSaturation(qreal s)
{
    sat = qMax(0.0, qMin(1.0, s));
    update();
}

void ColorSquare::setValue(qreal v)
{
    val = qMax(0.0, qMin(1.0, v));
    update();
}

void ColorSquare::setCheckedColorSlot(char checked)
{
    colorChar = checked;
    update();
}

void ColorSquare::RenderRectangle()
{
    int width = this->width();
    int height = this->height();
    colorSquare = QImage(width, height, QImage::Format_RGB32);
    for(int i = 0; i < width; ++i)
    {
        for(int j = 0; j < height; ++j)
        {
            switch(colorChar)
            {
            case 'S':
                colorSquare.setPixel(i,j,QColor::fromHsvF(double(i)/width, sat, double(j)/height).rgb());
                break;
            case 'V':
                colorSquare.setPixel(i,j,QColor::fromHsvF(double(i)/width, double(j)/height, val).rgb());
                break;
            case 'R':
            {
                qreal r = QColor::fromHsvF(huem, sat, val).redF();
                colorSquare.setPixel(i,j,QColor::fromRgbF(r, double(i)/width, double(j)/height).rgb());
                break;
            }
            case 'G':
            {
                qreal g = QColor::fromHsvF(huem, sat, val).greenF();
                colorSquare.setPixel(i,j,QColor::fromRgbF(double(i)/width, g, double(j)/height).rgb());
                break;
            }
            case 'B':
            {
                qreal b = QColor::fromHsvF(huem, sat, val).blueF();
                colorSquare.setPixel(i,j,QColor::fromRgbF(double(i)/width, double(j)/height, b).rgb());
                break;
            }
            default:
            {
                colorSquare.setPixel(i,j,QColor::fromHsvF(huem, double(i)/width, double(j)/height).rgb());
                break;
            }
            }
        }
    }
}

void ColorSquare::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    RenderRectangle();

    painter.setPen(Qt::NoPen);

    painter.drawImage(QRectF(1, 1, this->width(), this->height()), colorSquare);
    double selectorWidth = 6;//圆形大小
    painter.setPen(QPen(Qt::white, 3));
    painter.setBrush(Qt::NoBrush);
    // 绘制圆形光标
    painter.drawEllipse(QPointF(sat * this->width(), val * this->height()),
                        selectorWidth, selectorWidth);
}

void ColorSquare::mousePressEvent(QMouseEvent *ev)
{
    if(ev->buttons() & Qt::LeftButton)
    {
        mouseStatus = DragSquare;
        this->setFocus();
    }
}

void ColorSquare::mouseReleaseEvent(QMouseEvent *ev)
{
    mouseMoveEvent(ev);
    mouseStatus = Nothing;
}

void ColorSquare::mouseMoveEvent(QMouseEvent *ev)
{
    QLineF glob_mouse_ln = lineToPoint(ev->pos());
    QLineF center_mouse_ln ( QPointF(0,0),
                             glob_mouse_ln.p2() - glob_mouse_ln.p1() );

    colorX = center_mouse_ln.x2()/this->width() +0.5;

    colorY = center_mouse_ln.y2()/this->height() +0.5;

    if ( colorX > 1 )
        colorX = 1;
    else if ( colorX < 0 )
        colorX = 0;

    if ( colorY > 1 )
        colorY = 1;
    else if ( colorY < 0 )
        colorY = 0;

    switch(colorChar)
    {
    case 'H':
    {
        sat = colorX;
        val = colorY;
        break;
    }
    case 'S':
    {
        huem = colorX;
        val = colorY;
        break;
    }
    case 'V':
    {
        huem = colorX;
        sat = colorY;
        break;
    }
    case 'R':
    {
        qreal r = QColor::fromHsvF(huem,sat,val).redF();
        huem = QColor::fromRgbF(r, colorX, colorY).hueF();
        sat = QColor::fromRgbF(r, colorX, colorY).saturationF();
        val = QColor::fromRgbF(r, colorX, colorY).valueF();
        break;
    }
    case 'G':
    {
        qreal g = QColor::fromHsvF(huem,sat,val).greenF();
        huem = QColor::fromRgbF(colorX, g, colorY).hueF();
        sat = QColor::fromRgbF(colorX, g, colorY).saturationF();
        val = QColor::fromRgbF(colorX, g, colorY).valueF();
        break;
    }
    case 'B':
    {
        qreal b = QColor::fromHsvF(huem,sat,val).blueF();
        huem = QColor::fromRgbF(colorX, colorY, b).hueF();
        sat = QColor::fromRgbF(colorX, colorY, b).saturationF();
        val = QColor::fromRgbF(colorX, colorY, b).valueF();
        break;
    }
    default:
    {
        sat = colorX;
        val = colorY;
        break;
    }
    }

    Q_EMIT colorSelected(color());
    Q_EMIT colorChanged(color());
    update();
}

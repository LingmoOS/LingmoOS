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
#ifndef COLORSQUARE_H
#define COLORSQUARE_H

#include <QFrame>
#include <QColor>
#include <qmath.h>
#include <QPainter>
#include <QMouseEvent>

class ColorSquare : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged DESIGNABLE true STORED false )
    Q_PROPERTY(qreal hue READ hue WRITE setHue DESIGNABLE false )
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation DESIGNABLE false )
    Q_PROPERTY(qreal value READ value WRITE setValue DESIGNABLE false )

public:
    explicit ColorSquare(QWidget *parent = 0);

    QColor color() const;
    qreal hue() const {return huem;}
    qreal saturation() const {return sat;}
    qreal value() const {return val;}

public Q_SLOTS:
    void setColor(QColor c);
    void setHue(qreal h);
    void setSaturation(qreal s);
    void setValue(qreal v);

    void setCheckedColorSlot(char checked);

Q_SIGNALS:
    void colorChanged(QColor);
    void colorSelected(QColor);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    qreal huem, sat, val;
    qreal colorX, colorY;
    QImage colorSquare;
    char    colorChar;

    enum CheckedColor
    {
        H,S,V,R,G,B
    }checkedColor;

    enum MouseStatus
    {
        Nothing,
        DragSquare
    }mouseStatus;

private:
    void RenderRectangle();

    QLineF lineToPoint(QPoint p) const
    {
        return QLineF(geometry().width()/2, geometry().height()/2, p.x(), p.y());
    }
};

#endif // COLORSQUARE_H

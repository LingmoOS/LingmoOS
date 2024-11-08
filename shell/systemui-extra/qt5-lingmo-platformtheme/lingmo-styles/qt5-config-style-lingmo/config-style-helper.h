/*
 * Qt5-LINGMO's Library
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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef CONFIGSTYLEHELPER_H
#define CONFIGSTYLEHELPER_H

#include <QStyleOption>
#include <qmath.h>

    const QRegion configGetRoundedRectRegion(const QRect &rect, qreal radius_x, qreal radius_y);
    qreal configCalcRadial(const QStyleOptionSlider *dial, int postion);
    QPolygonF configCalcLines(const QStyleOptionSlider *dial, int offset);
    void configTabLayout(const QStyleOptionTab *tab, const QWidget *widget, const QStyle *style, QRect *textRect, QRect *iconRect);
    QColor configMixColor(const QColor &c1, const QColor &c2, qreal bias = 0.5);
    void configDrawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton, const QRect &rect, QPainter *painter, const QWidget *widget = 0);

#endif // CONFIGSTYLEHELPER_H

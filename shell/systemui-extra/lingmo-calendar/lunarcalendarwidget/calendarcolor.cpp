/*
* Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "calendarcolor.h"
#include <QApplication>
#include <QPalette>
#include <QDebug>
namespace CalendarColor {


CalendarColor::CalendarColor()
{

}

QColor CalendarColor::getThemeColor(Color c)
{
    QColor color;
    QPalette pal = QApplication::palette();
    switch (c){
    case TEXT:{
        color = pal.text().color();
    }break;
    case BACKGROUND:{
        color = pal.background().color();
    }break;
    case CLICKED:{
        color = pal.highlight().color();
    }break;
    case CLICKED_TEXT:{
        color = pal.highlightedText().color();
    }break;
    case OTHER_TEXT:{
        color = pal.placeholderText().color();
    }break;
    case WORK:{
        color = QColor(255, 129, 6);
    }break;
    case HOLIDAY:{
        color = QColor(233, 90, 84);
    }break;
    default:{
        qDebug () << "Failed to get color, invalid color type";
        color = QColor();
    }
    }
//    qDebug() << (int)c << color;
    return color;
}

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b-a)*bias;
}

QColor CalendarColor::mixColor(const QColor& in,qreal mix)
{
    if (mix <= 0.0){
        return in;
    } else if(mix >= 1.0){
        return in;
    } else if(qIsNaN(mix)) {
        return in;
    }

    QColor brightTextColor = QApplication::palette().brightText().color();
    qreal r = mixQreal(in.redF(),brightTextColor.redF(),mix);
    qreal g = mixQreal(in.greenF(),brightTextColor.greenF(),mix);
    qreal b = mixQreal(in.blueF(),brightTextColor.blueF(),mix);
    qreal a = mixQreal(in.alphaF(),brightTextColor.alphaF(),mix);
    return QColor::fromRgbF(r, g, b, a);
}
}

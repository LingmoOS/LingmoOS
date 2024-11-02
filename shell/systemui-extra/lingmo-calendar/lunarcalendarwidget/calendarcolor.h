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

#ifndef CALENDARCOLOR_H
#define CALENDARCOLOR_H

#include <QColor>

namespace  CalendarColor {
    enum Color{
        TEXT = 1, //Text
        BACKGROUND = 2, //Background
        CLICKED =3, //Highlight
        CLICKED_TEXT = 4, //HighlightText
        OTHER_TEXT = 5, //Shadow
        HOLIDAY = 6, //Red
        WORK = 7 //Orange
    };


class CalendarColor
{
public:
    CalendarColor();
public:
    static QColor getThemeColor(Color c);
    //混合颜色，参考主题代码
    static QColor mixColor(const QColor& in,qreal mix);
};

}
#endif // CALENDARCOLOR_H

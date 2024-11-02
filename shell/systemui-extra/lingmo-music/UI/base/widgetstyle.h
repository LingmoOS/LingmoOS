/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef WIDGETSTYLE_H
#define WIDGETSTYLE_H

#include <QGSettings>

#define LINGMOMUSIC "org.lingmo-music-data.settings"
#define FITTHEMEWINDOWS "org.lingmo.style"

//static int themeColor = 0;   //test  主题颜色适配--浅色
//int WidgetStyle::themeColor = 1;   //test  主题颜色适配--深色
class WidgetStyle
{
public:
    WidgetStyle() {}

    static int themeColor;   //test  主题颜色适配--浅色
    static bool isWayland;
//    const QColor highlightTextColor;

};





#endif // WIDGETSTYLE_H

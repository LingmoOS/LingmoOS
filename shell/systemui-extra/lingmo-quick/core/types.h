/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ITEMS_TYPES_H
#define LINGMO_QUICK_ITEMS_TYPES_H

#include <QMetaObject>

namespace LingmoUIQuick {

class Types
{
    Q_GADGET
public:
    enum Pos {
        NoPosition = 0,
        Left,
        Top,
        Right,
        Bottom,
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft,
        Center,
        LeftCenter,
        TopCenter,
        RightCenter,
        BottomCenter
    };
    Q_ENUM(Pos)

    enum Orientation {
        Vertical,
        Horizontal
    };
    Q_ENUM(Orientation)

    enum WidgetType {
        Widget,
        Container
    };
    Q_ENUM(WidgetType)

    /**
     * The Location enumeration describes where on screen an element, such as an
     * Applet or its managing container, is positioned on the screen.
     **/
    enum PopupLocation {
        Floating = 0, /**< Free floating. Neither geometry or z-ordering
                     is described precisely by this value. */
        Desktop, /**< On the planar desktop layer, extending across
                the full screen from edge to edge */
        FullScreen, /**< Full screen */
        TopEdge, /**< Along the top of the screen*/
        BottomEdge, /**< Along the bottom of the screen*/
        LeftEdge, /**< Along the left side of the screen */
        RightEdge, /**< Along the right side of the screen */
    };
    Q_ENUM(PopupLocation)
};

} // global

#endif //LINGMO_QUICK_ITEMS_TYPES_H

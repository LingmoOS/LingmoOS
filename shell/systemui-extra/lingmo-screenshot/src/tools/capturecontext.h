/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
 *              2020 KylinSoft Co., Ltd.
 * This file is part of Lingmo-Screenshot.
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

#pragma once

#include <QRect>
#include <QPoint>
#include <QPixmap>
#include <QPainter>
#include <QGSettings/qgsettings.h>
struct CaptureContext {
    // screenshot with modifications
    QPixmap screenshot;
    // unmodified screenshot
    QPixmap origScreenshot;
    // Selection area
    QRect selection;
    // Widget dimensions
    QRect widgetDimensions;
    // Selected tool color
    QColor color;
    // Path where the content has to be saved
    QString savePath;
    // Ofset of the capture widget based on the system's screen (top-left)
    QPoint widgetOffset;
    // Mouse position inside the widget
    QPoint mousePos;
    // Value of the desired thickness
    int thickness;
    int text_thickness;
    // Mode of the capture widget
    bool fullscreen;

    QString  saveType;

    QPixmap selectedScreenshotArea() const ;
    QFont font_type;
    bool bold;
    bool underline;
    bool deleteline;
    bool italic;   
#ifdef SUPPORT_LINGMO
    QGSettings *style_settings;
    QString style_name;
#endif
};

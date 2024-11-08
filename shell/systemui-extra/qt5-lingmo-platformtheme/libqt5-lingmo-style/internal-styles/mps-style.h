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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef MPSSTYLE_H
#define MPSSTYLE_H

#include "internal-style.h"

/*!
 * \brief The MPSStyle class
 * \details
 * M(enu)P(anel)S(idebar)Style is an internal style shared in several
 * LINGMO desktop applications, such as LINGMO Menu, LINGMO Panel and LINGMO Sidebar, etc.
 *
 * Usually, applications use the styles which have been installed as qstyle plugin in
 * LINGMO desktop environment, such as lingmo-white and lingmo-black. However one style may
 * be not enough for LINGMO3.0 desgin. The menu, panel and sidebar desgin is Noticeably
 * different from the common style which we used in explorer or other qt's applications.
 * To integrate the unique style into the theme, I came up the idea of internal styles,
 * and combine them with current platform theme's frameworks. So it will Looks like a
 * whole style.
 *
 * \see InternalStyle
 */
class MPSStyle : public InternalStyle
{
    Q_OBJECT
public:
    explicit MPSStyle(bool dark = true);

    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const;
    void drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const;
    void drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const;

    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const;
    QStyle::SubControl hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget = nullptr) const;
    QRect itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const;
    QRect itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text) const;
    //virtual int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget);
    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const;

    void polish(QWidget *widget);
    void polish(QApplication *application);
    void polish(QPalette &palette);
    void unpolish(QWidget *widget);
    void unpolish(QApplication *application);

    QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget = nullptr) const;
    QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const;
    QPalette standardPalette() const;
    int	styleHint(QStyle::StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const;
    QRect subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget = nullptr) const;
    QRect subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget = nullptr) const;
};

#endif // MPSSTYLE_H

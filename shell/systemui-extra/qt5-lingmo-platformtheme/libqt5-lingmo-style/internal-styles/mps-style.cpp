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

#include "mps-style.h"

MPSStyle::MPSStyle(bool dark) : InternalStyle ("fusion")
{

}

void MPSStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    //FIXME:
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void MPSStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    //FIXME:
    QProxyStyle::drawControl(element, option, painter, widget);
}

void MPSStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    //FIXME:
    QProxyStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void MPSStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    //FIXME:
    QProxyStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void MPSStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    //FIXME:
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QPixmap MPSStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
{
    //FIXME:
    return QProxyStyle::generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl MPSStyle::hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const
{
    //FIXME:
    return QProxyStyle::hitTestComplexControl(control, option, position, widget);
}

QRect MPSStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    //FIXME:
    return QProxyStyle::itemPixmapRect(rectangle, alignment, pixmap);
}

QRect MPSStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text) const
{
    //FIXME:
    return QProxyStyle::itemTextRect(metrics, rectangle, alignment, enabled, text);
}

int MPSStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    //FIXME:
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void MPSStyle::polish(QWidget *widget)
{
    //FIXME:
    QProxyStyle::polish(widget);
}

void MPSStyle::polish(QApplication *application)
{
    //FIXME:
    QProxyStyle::polish(application);
}

void MPSStyle::polish(QPalette &palette)
{
    //FIXME:
    QProxyStyle::polish(palette);
}

void MPSStyle::unpolish(QWidget *widget)
{
    //FIXME:
    QProxyStyle::unpolish(widget);
}

void MPSStyle::unpolish(QApplication *application)
{
    //FIXME:
    QProxyStyle::unpolish(application);
}

QSize MPSStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    //FIXME:
    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}

QIcon MPSStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    //FIXME:
    return QProxyStyle::standardIcon(standardIcon, option, widget);
}

QPalette MPSStyle::standardPalette() const
{
    //FIXME:
    return QProxyStyle::standardPalette();
}

int MPSStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    //FIXME:
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QRect MPSStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    //FIXME:
    return QProxyStyle::subControlRect(control, option, subControl, widget);
}

QRect MPSStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    //FIXME:
    return QProxyStyle::subElementRect(element, option, widget);
}

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include "customstyle.h"

CustomStyle::CustomStyle(const QString &proxyStyleName) : QProxyStyle (proxyStyleName)
{

}

QSize CustomStyle::sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    switch (type) {
        case CT_TabBarTab: {
            QSize size(0,40);
            if (const QStyleOptionTab *tab= qstyleoption_cast<const QStyleOptionTab *>(option)) {
                switch (tab->shape) {
                    case QTabBar::RoundedNorth:
                    case QTabBar::RoundedSouth: {
                        return size + QSize(50,0);
                        break;
                    }
                    case QTabBar::RoundedWest:
                    case QTabBar::RoundedEast: {
                        return size + QSize(0,50);
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
            return size;
            break;
        }
        default: {
            break;
        }
    }
    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}
void CustomStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
//    switch (element) {
//        case CE_TabBarTab: {
//            if (QStyle::State_Selected != option->state) {
//                painter->save();
//                painter->setBrush(QColor(0,0,0,0));
//                painter->drawRect(widget->rect());
////                painter->setOpacity(0.1);
//                painter->restore();
//            }
//            break;
//        }
//        default: {
//            break;
//        }
//    }
    return QProxyStyle::drawControl(element, option, painter, widget);
}

//void CustomStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
//{
//    return QProxyStyle::drawComplexControl(control, option, painter, widget);
//}

//void CustomStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const
//{
//    return QProxyStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
//}

//void CustomStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
//{
//    return QProxyStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
//}

//void CustomStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
//{
//    return QProxyStyle::drawPrimitive(element, option, painter, widget);
//}

//QPixmap CustomStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
//{
//    return QProxyStyle::generatedIconPixmap(iconMode, pixmap, option);
//}

//QStyle::SubControl CustomStyle::hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const
//{
//    return QProxyStyle::hitTestComplexControl(control, option, position, widget);
//}

//QRect CustomStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const
//{
//    return QProxyStyle::itemPixmapRect(rectangle, alignment, pixmap);
//}

//QRect CustomStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text) const
//{
//    return QProxyStyle::itemTextRect(metrics, rectangle, alignment, enabled, text);
//}

//int CustomStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
//{
//    return QProxyStyle::pixelMetric(metric, option, widget);
//}

//void CustomStyle::polish(QWidget *widget)
//{
//    return QProxyStyle::polish(widget);
//}

//void CustomStyle::polish(QApplication *application)
//{
//    return QProxyStyle::polish(application);
//}

//void CustomStyle::polish(QPalette &palette)
//{
//    return QProxyStyle::polish(palette);
//}

//void CustomStyle::unpolish(QWidget *widget)
//{
//    return QProxyStyle::unpolish(widget);
//}

//void CustomStyle::unpolish(QApplication *application)
//{
//    return QProxyStyle::unpolish(application);
//}

//QIcon CustomStyle::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
//{
//    return QProxyStyle::standardIcon(standardIcon, option, widget);
//}

//QPalette CustomStyle::standardPalette() const
//{
//    return QProxyStyle::standardPalette();
//}

//int CustomStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
//{
//    return QProxyStyle::styleHint(hint, option, widget, returnData);
//}

//QRect CustomStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
//{
//    return QProxyStyle::subControlRect(control, option, subControl, widget);
//}

//QRect CustomStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
//{
//    return QProxyStyle::subElementRect(element, option, widget);
//}

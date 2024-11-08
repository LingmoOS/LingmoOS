/*
 * KWin Style LINGMO
 *
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"
#include "menutoolbutoon.h"
#include <QApplication>
#include <QDBusInterface>
using namespace LINGMOFileDialog;

static ToolButtonStyle *global_instance = nullptr;

MenuToolButton::MenuToolButton(QWidget *parent) : QToolButton(parent)
{
    setStyle(ToolButtonStyle::getStyle());

    QDBusInterface *interFace = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus());
    if(interFace->isValid()){
        connect(interFace, SIGNAL(mode_change_signal(bool)), this,  SLOT(tableModeChanged(bool)));
    }
}

void MenuToolButton::tableModeChanged(bool isTableMode)
{
    setStyle(ToolButtonStyle::getStyle());
}

ToolButtonStyle *ToolButtonStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new ToolButtonStyle;
    }
    return global_instance;
}

int ToolButtonStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (qobject_cast<const QToolButton *>(widget))
        return 0;

    switch (metric) {
    case PM_ToolBarIconSize:
        return 16;
    case PM_ToolBarSeparatorExtent:
        return 1;
    case PM_ToolBarItemSpacing: {
        return 1;
    }
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void ToolButtonStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    //This is a "lie". We want to use instant popup menu for tool button, and we aslo
    //want use popup menu style with this tool button, so we change the related flags
    //to draw in our expected.
    if (control == CC_ToolButton) {
        QStyleOptionToolButton button = *qstyleoption_cast<const QStyleOptionToolButton *>(option);
        if (button.features.testFlag(QStyleOptionToolButton::HasMenu)) {
            button.features = QStyleOptionToolButton::None;
            if (!widget->property("isOptionButton").toBool()) {
                button.features |= QStyleOptionToolButton::HasMenu;
                button.features |= QStyleOptionToolButton::MenuButtonPopup;
                button.subControls |= QStyle::SC_ToolButtonMenu;
            }
            return qApp->style()->drawComplexControl(control, &button, painter, widget);
        }
    }
    return qApp->style()->drawComplexControl(control, option, painter, widget);
}

void ToolButtonStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == PE_IndicatorToolBarSeparator) {
        return;
    }
    return qApp->style()->drawPrimitive(element, option, painter, widget);
}

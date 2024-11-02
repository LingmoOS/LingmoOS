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

#ifndef MENUTOOLBUTTON_H
#define MENUTOOLBUTTON_H
#include <QToolButton>
#include <QProxyStyle>
#include <QStyleOption>
#include <QPainter>
namespace LINGMOFileDialog {
class MenuToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit MenuToolButton(QWidget *parent = nullptr);
    ~MenuToolButton(){}

Q_SIGNALS:


public Q_SLOTS:
    void tableModeChanged(bool isTableMode);

};

class ToolButtonStyle : public QProxyStyle
{
    friend class MenuToolButton;
    friend class HeaderBarContainer;
    static ToolButtonStyle *getStyle();

    ToolButtonStyle() {}
    ~ToolButtonStyle() {}

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
};
}
#endif // MENUTOOLBUTTON_H

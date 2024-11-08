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
 * Authors: xibowen <lanyue@kylinos.cn>
 *
 */



#ifndef PUSHBUTTONPARAMETERS_H
#define PUSHBUTTONPARAMETERS_H

#include <QBrush>
#include <QPen>
#include "control-parameters.h"

namespace LINGMOConfigStyleSpace {

class ConfigPushButtonParameters : public ControlParameters
{
    Q_OBJECT
public:
    ConfigPushButtonParameters();

//private:
    int radius;

    bool iconHoverClickHighlight;
    bool textHoverClickHighlight;
    QColor textDefaultColor;
    QColor textHoverColor;
    QColor textDisableColor;


    QBrush pushButtonDefaultBrush;
    QBrush pushButtonHoverBrush;
    QBrush pushButtonClickBrush;
    QBrush pushButtonDisableBrush;
    QBrush pushButtonCheckBrush;
    QBrush pushButtonCheckHoverBrush;
    QBrush pushButtonCheckClickBrush;
    QBrush pushButtonCheckDisableBrush;

    QPen pushButtonDefaultPen;
    QPen pushButtonHoverPen;
    QPen pushButtonClickPen;
    QPen pushButtonDisablePen;
    QPen pushButtonCheckPen;
    QPen pushButtonCheckHoverPen;
    QPen pushButtonCheckClickPen;
    QPen pushButtonCheckDisablePen;
    QPen pushButtonFocusPen;

    bool needPen = false;


//    QColor PushButtonInactiveColor;

//    QColor PushButtonTextDefaultColor;
//    QColor PushButtonTextHoverColor;
//    QColor PushButtonTextClickColor;
//    QColor PushButtonTextInactiveColor;
//    QColor PushButtonTextDisableColor;
};
}
#endif // PUSHBUTTONPARAMETERS_H

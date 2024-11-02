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


#ifndef TOOLBUTTONPARAMETERS_H
#define TOOLBUTTONPARAMETERS_H

#include <QBrush>
#include <QPen>
#include "control-parameters.h"

namespace LINGMOConfigStyleSpace {

class ConfigToolButtonParameters : public ControlParameters
{
    Q_OBJECT
public:
    ConfigToolButtonParameters();

//private:
    int radius;

    bool iconHoverClickHighlight;
    bool textHoverClickHighlight;

    QBrush toolButtonDefaultBrush;
    QBrush toolButtonHoverBrush;
    QBrush toolButtonClickBrush;
    QBrush toolButtonDisableBrush;
    QBrush toolButtonCheckBrush;
    QBrush toolButtonCheckHoverBrush;
    QBrush toolButtonCheckClickBrush;
    QBrush toolButtonCheckDisableBrush;

    QPen toolButtonDefaultPen;
    QPen toolButtonHoverPen;
    QPen toolButtonClickPen;
    QPen toolButtonDisablePen;
    QPen toolButtonCheckPen;
    QPen toolButtonCheckHoverPen;
    QPen toolButtonCheckClickPen;
    QPen toolButtonCheckDisablePen;
    QPen toolButtonFocusPen;

    QColor textDefaultColor;
    QColor textHoverColor  ;
    QColor textDisableColor;
};
}
#endif // TOOLBUTTONPARAMETERS_H

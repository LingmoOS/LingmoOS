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


#ifndef SPINBOXPARAMETERS_H
#define SPINBOXPARAMETERS_H

#include <QBrush>
#include <QPen>
#include "control-parameters.h"
namespace LINGMOConfigStyleSpace {

class ConfigSpinBoxParameters : public ControlParameters
{
    Q_OBJECT
public:
    ConfigSpinBoxParameters();

//private:
    int radius;

    QBrush spinBoxDefaultBrush;
    QBrush spinBoxHoverBrush;
    QBrush spinBoxFocusBrush;
    QBrush spinBoxDisableBrush;
    QPen spinBoxDefaultPen;
    QPen spinBoxHoverPen;
    QPen spinBoxFocusPen;
    QPen spinBoxDisablePen;

    //up button
    QBrush spinBoxUpDefaultBrush;
    QBrush spinBoxUpHoverBrush;
    QBrush spinBoxUpFocusHoverBrush;
    QBrush spinBoxUpClickBrush;
    QBrush spinBoxUpDisableBrush;
    QPen spinBoxUpDefaultPen;
    QPen spinBoxUpHoverPen;
    QPen spinBoxUpFocusHoverPen;
    QPen spinBoxUpClickPen;
    QPen spinBoxUpDisablePen;

    //down button
    QBrush spinBoxDownDefaultBrush;
    QBrush spinBoxDownHoverBrush;
    QBrush spinBoxDownFocusHoverBrush;
    QBrush spinBoxDownClickBrush;
    QBrush spinBoxDownDisableBrush;
    QPen spinBoxDownDefaultPen;
    QPen spinBoxDownHoverPen;
    QPen spinBoxDownFocusHoverPen;
    QPen spinBoxDownClickPen;
    QPen spinBoxDownDisablePen;

    bool spinBoxUpIconHightPixMap = true;
    bool spinBoxDownIconHightPixMap = true;
};
}
#endif // SPINBOXPARAMETERS_H

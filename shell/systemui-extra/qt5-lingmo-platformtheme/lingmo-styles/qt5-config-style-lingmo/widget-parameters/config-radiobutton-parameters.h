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

#ifndef RADIOBUTTONPARAMETERS_H
#define RADIOBUTTONPARAMETERS_H

#include <QBrush>
#include <QPen>
#include "control-parameters.h"

namespace LINGMOConfigStyleSpace {

class ConfigRadioButtonParameters : public ControlParameters
{
    Q_OBJECT
public:
    ConfigRadioButtonParameters();

//private:
    int radius;
    int childrenRadius;

    QBrush radioButtonDefaultBrush;
    QBrush radioButtonHoverBrush;
    QBrush radioButtonClickBrush;
    QBrush radioButtonDisableBrush;
    QBrush radioButtonOnDefaultBrush;
    QBrush radioButtonOnHoverBrush;
    QBrush radioButtonOnClickBrush;
    QBrush radioButtonChildrenOnDefaultBrush;
    QBrush radioButtonChildrenOnHoverBrush;
    QBrush radioButtonChildrenOnClickBrush;
    QBrush radioButtonChildrenOnDisableBrush;

    QPen radioButtonDefaultPen;
    QPen radioButtonHoverPen;
    QPen radioButtonClickPen;
    QPen radioButtonDisablePen;
    QPen radioButtonOnDefaultPen;
    QPen radioButtonOnHoverPen;
    QPen radioButtonOnClickPen;
//    bool animation = true;
};
}
#endif // RADIOBUTTONPARAMETERS_H

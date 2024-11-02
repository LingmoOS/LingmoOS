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


#ifndef SLIDERPARAMETERS_H
#define SLIDERPARAMETERS_H

#include <QBrush>
#include <QPainterPath>
#include <QPen>
#include "control-parameters.h"

namespace LINGMOConfigStyleSpace {

class ConfigSliderParameters : public ControlParameters
{
    Q_OBJECT
public:
    ConfigSliderParameters();

//private:
    int radius;
    QBrush sliderHandleDefaultBrush;
    QBrush sliderHandleHoverBrush;
    QBrush sliderHandleClickBrush;
    QBrush sliderHandleDisableBrush;

    QBrush sliderGrooveValueDefaultBrush;
    QBrush sliderGrooveValueHoverBrush;
    QBrush sliderGrooveValueDisableBrush;

    QBrush sliderGrooveUnvalueDefaultBrush;
    QBrush sliderGrooveUnvalueHoverBrush;
    QBrush sliderGrooveUnvalueDisableBrush;

    QPen sliderHandleDefaultPen = Qt::NoPen;
    QPen sliderHandleHoverPen = Qt::NoPen;
    QPen sliderHandleClickPen = Qt::NoPen;
    QPen sliderHandleDisablePen = Qt::NoPen;
    QPen sliderGroovePen = Qt::NoPen;
    QPen focusPen = Qt::NoPen;
    QPainterPath sliderHandlePath;

//    bool animation = true;

    int sliderGrooveRadius = 2;
};
}
#endif // SLIDERPARAMETERS_H

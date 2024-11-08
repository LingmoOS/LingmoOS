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


#include "config-spinbox-parameters.h"
using namespace LINGMOConfigStyleSpace;

ConfigSpinBoxParameters::ConfigSpinBoxParameters()
{
    radius = 0;

    spinBoxDefaultBrush = QBrush(Qt::NoBrush);
    spinBoxHoverBrush = QBrush(Qt::NoBrush);
    spinBoxFocusBrush = QBrush(Qt::NoBrush);
    spinBoxDisableBrush = QBrush(Qt::NoBrush);
    spinBoxDefaultPen = QPen(Qt::NoPen);
    spinBoxHoverPen = QPen(Qt::NoPen);
    spinBoxFocusPen = QPen(Qt::NoPen);
    spinBoxDisablePen = QPen(Qt::NoPen);
    spinBoxDefaultPen.setWidth(0);
    spinBoxHoverPen.setWidth(0);
    spinBoxFocusPen.setWidth(0);
    spinBoxDisablePen.setWidth(0);

    //up button
    spinBoxUpDefaultBrush = QBrush(Qt::NoBrush);
    spinBoxUpHoverBrush = QBrush(Qt::NoBrush);
    spinBoxUpFocusHoverBrush = QBrush(Qt::NoBrush);
    spinBoxUpClickBrush = QBrush(Qt::NoBrush);
    spinBoxUpDisableBrush = QBrush(Qt::NoBrush);
    spinBoxUpDefaultPen = QPen(Qt::NoPen);
    spinBoxUpHoverPen = QPen(Qt::NoPen);
    spinBoxUpFocusHoverPen = QPen(Qt::NoPen);
    spinBoxUpClickPen = QPen(Qt::NoPen);
    spinBoxUpDisablePen = QPen(Qt::NoPen);
    spinBoxUpDefaultPen.setWidth(0);
    spinBoxUpHoverPen.setWidth(0);
    spinBoxUpFocusHoverPen.setWidth(0);
    spinBoxUpClickPen.setWidth(0);
    spinBoxUpDisablePen.setWidth(0);

    //down button
    spinBoxDownDefaultBrush = QBrush(Qt::NoBrush);
    spinBoxDownHoverBrush = QBrush(Qt::NoBrush);
    spinBoxDownFocusHoverBrush = QBrush(Qt::NoBrush);
    spinBoxDownClickBrush = QBrush(Qt::NoBrush);
    spinBoxDownDisableBrush = QBrush(Qt::NoBrush);
    spinBoxDownDefaultPen = QPen(Qt::NoPen);
    spinBoxDownHoverPen = QPen(Qt::NoPen);
    spinBoxDownFocusHoverPen = QPen(Qt::NoPen);
    spinBoxDownClickPen = QPen(Qt::NoPen);
    spinBoxDownDisablePen = QPen(Qt::NoPen);
    spinBoxDownDefaultPen.setWidth(0);
    spinBoxDownHoverPen.setWidth(0);
    spinBoxDownFocusHoverPen.setWidth(0);
    spinBoxDownClickPen.setWidth(0);
    spinBoxDownDisablePen.setWidth(0);
}

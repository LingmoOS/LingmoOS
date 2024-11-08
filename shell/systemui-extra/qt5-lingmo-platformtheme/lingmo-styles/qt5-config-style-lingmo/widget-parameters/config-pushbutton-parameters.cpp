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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include "config-pushbutton-parameters.h"
using namespace LINGMOConfigStyleSpace;

ConfigPushButtonParameters::ConfigPushButtonParameters()
{
    radius = 0;

    iconHoverClickHighlight = false;
    textHoverClickHighlight = false;

    pushButtonDefaultBrush = QBrush(Qt::NoBrush);
    pushButtonHoverBrush = QBrush(Qt::NoBrush);
    pushButtonClickBrush = QBrush(Qt::NoBrush);
    pushButtonDisableBrush = QBrush(Qt::NoBrush);
    pushButtonCheckBrush = QBrush(Qt::NoBrush);
    pushButtonCheckHoverBrush = QBrush(Qt::NoBrush);
    pushButtonCheckClickBrush = QBrush(Qt::NoBrush);
    pushButtonCheckDisableBrush = QBrush(Qt::NoBrush);

    pushButtonDefaultPen = QPen(Qt::NoPen);
    pushButtonHoverPen = QPen(Qt::NoPen);
    pushButtonClickPen = QPen(Qt::NoPen);
    pushButtonDisablePen = QPen(Qt::NoPen);
    pushButtonCheckPen = QPen(Qt::NoPen);
    pushButtonCheckHoverPen = QPen(Qt::NoPen);
    pushButtonCheckClickPen = QPen(Qt::NoPen);
    pushButtonCheckDisablePen = QPen(Qt::NoPen);
    pushButtonFocusPen = QPen(Qt::NoPen);
    pushButtonDefaultPen.setWidth(0);
    pushButtonHoverPen.setWidth(0);
    pushButtonClickPen.setWidth(0);
    pushButtonDisablePen.setWidth(0);
    pushButtonCheckPen.setWidth(0);
    pushButtonCheckHoverPen.setWidth(0);
    pushButtonCheckClickPen.setWidth(0);
    pushButtonCheckDisablePen.setWidth(0);
    pushButtonFocusPen.setWidth(0);
}

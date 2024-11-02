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

#include "config-toolbutton-parameters.h"
using namespace LINGMOConfigStyleSpace;

ConfigToolButtonParameters::ConfigToolButtonParameters()
{
    radius = 0;

    iconHoverClickHighlight = false;
    textHoverClickHighlight = false;

    toolButtonDefaultBrush = QBrush(Qt::NoBrush);
    toolButtonHoverBrush = QBrush(Qt::NoBrush);
    toolButtonClickBrush = QBrush(Qt::NoBrush);
    toolButtonDisableBrush = QBrush(Qt::NoBrush);
    toolButtonCheckBrush = QBrush(Qt::NoBrush);
    toolButtonCheckHoverBrush = QBrush(Qt::NoBrush);
    toolButtonCheckClickBrush = QBrush(Qt::NoBrush);
    toolButtonCheckDisableBrush = QBrush(Qt::NoBrush);

    toolButtonDefaultPen = QPen(Qt::NoPen);
    toolButtonHoverPen = QPen(Qt::NoPen);
    toolButtonClickPen = QPen(Qt::NoPen);
    toolButtonDisablePen = QPen(Qt::NoPen);
    toolButtonCheckPen = QPen(Qt::NoPen);
    toolButtonCheckHoverPen = QPen(Qt::NoPen);
    toolButtonCheckClickPen = QPen(Qt::NoPen);
    toolButtonCheckDisablePen = QPen(Qt::NoPen);
    toolButtonFocusPen = QPen(Qt::NoPen);
    toolButtonDefaultPen.setWidth(0);
    toolButtonHoverPen.setWidth(0);
    toolButtonClickPen.setWidth(0);
    toolButtonDisablePen.setWidth(0);
    toolButtonCheckPen.setWidth(0);
    toolButtonCheckHoverPen.setWidth(0);
    toolButtonCheckClickPen.setWidth(0);
    toolButtonCheckDisablePen.setWidth(0);
    toolButtonFocusPen.setWidth(0);
}

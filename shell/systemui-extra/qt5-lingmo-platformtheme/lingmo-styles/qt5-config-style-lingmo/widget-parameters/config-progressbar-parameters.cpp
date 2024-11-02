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


#include "config-progressbar-parameters.h"
using namespace LINGMOConfigStyleSpace;

ConfigProgressBarParameters::ConfigProgressBarParameters()
{
    radius = 0;

    progressBarContentBrush = QBrush(Qt::NoBrush);
    progressBarGrooveDefaultBrush = QBrush(Qt::NoBrush);
    progressBarGrooveDisableBrush = QBrush(Qt::NoBrush);

    progressBarContentPen = QPen(Qt::NoPen);
    progressBarGrooveDefaultPen = QPen(Qt::NoPen);
    progressBarGrooveDisablePen = QPen(Qt::NoPen);
    progressBarContentPen.setWidth(0);
    progressBarGrooveDefaultPen.setWidth(0);
    progressBarGrooveDisablePen.setWidth(0);
}

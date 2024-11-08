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

#include "config-combobox-parameters.h"
using namespace LINGMOConfigStyleSpace;

ConfigComboBoxParameters::ConfigComboBoxParameters()
{
    radius = 0;
    comboBoxDefaultBrush = QBrush(Qt::NoBrush);
    comboBoxHoverBrush = QBrush(Qt::NoBrush);
    comboBoxOnBrush = QBrush(Qt::NoBrush);
    comboBoxEditBrush = QBrush(Qt::NoBrush);
    comboBoxDisableBrush = QBrush(Qt::NoBrush);

    comboBoxDefaultPen = QPen(Qt::NoPen);
    comboBoxHoverPen = QPen(Qt::NoPen);
    comboBoxOnPen = QPen(Qt::NoPen);
    comboBoxEditPen = QPen(Qt::NoPen);
    comboBoxDisablePen = QPen(Qt::NoPen);
    comboBoxDefaultPen.setWidth(0);
    comboBoxHoverPen.setWidth(0);
    comboBoxOnPen.setWidth(0);
    comboBoxEditPen.setWidth(0);
    comboBoxDisablePen.setWidth(0);
}

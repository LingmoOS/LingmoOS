/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screen_power.h"

ScreenBrightNode::ScreenBrightNode(QObject *parent)
{
    qDebug() << "init screenBrightNode";
}

ScreenBrightNode::~ScreenBrightNode()
{

}

bool ScreenBrightNode::setScreenBrightness(QString name, uint screenBrightness)
{
    return false;
}

uint ScreenBrightNode::getScreenBrightness(QString name)
{
     return -1;
}

bool ScreenBrightNode::setScreensBrightness(OutputGammaInfoList outputsInfo)
{
    return false;
}

bool ScreenBrightNode::setAllScreenSameBrightness(uint brighenss)
{
   return false;
}

OutputGammaInfoList ScreenBrightNode::getAllScreenBrightness()
{
    OutputGammaInfoList ret;
    return ret;
}

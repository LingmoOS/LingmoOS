/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "setFontColorPage.h"

QString SetFontColor::LINGMO_BLUE = "#1D52E1";
QString SetFontColor::LINGMO_RED = "#EE0000";
QString SetFontColor::LINGMO_GREEN = "#00D600";
QString SetFontColor::LINGMO_ORANGE = "#E39900";
QString SetFontColor::LINGMO_PURPLE = "#FF2F96";

SetFontColor::SetFontColor(QWidget *parent) :
      CustomColorPanel(parent)
{
    initSetup();
}

SetFontColor::~SetFontColor()
{


}

void SetFontColor::initSetup()
{
    setWindowFlags(Qt::Popup);
    addColor(LINGMO_BLUE);
    addColor(LINGMO_RED);
    addColor(LINGMO_GREEN);
    addColor(LINGMO_ORANGE);
    addColor(LINGMO_PURPLE);
    addColor(CustomColorPanel::BLACK, true);
}

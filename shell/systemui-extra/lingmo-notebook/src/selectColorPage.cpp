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

#include <QPainterPath>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

#include "selectColorPage.h"


QString SelectColor::LINGMO_BLUE = "#3790FA";
QString SelectColor::LINGMO_PURPLE = "#722ED1";
QString SelectColor::LINGMO_PINK = "#EB3096";
QString SelectColor::LINGMO_RED = "#F3222D";
QString SelectColor:: LINGMO_ORANGE = "#F68C27";
QString SelectColor::LINGMO_YELLOW = "#F9C53D";
QString SelectColor::LINGMO_GREEN = "#52C429";
QString SelectColor::LINGMO_GREY = "#8C8C8C";

SelectColor::SelectColor(QWidget *parent) :
      CustomColorPanel(parent),
      m_parentWindow(parent)
{
    initSetup();
}

SelectColor::~SelectColor()
{

}

void SelectColor::initSetup()
{
//    setFixedSize(48, 272);
    setWindowFlags(Qt::Popup);
    addColor(LINGMO_BLUE, true);
    addColor(LINGMO_RED);
    addColor(LINGMO_GREEN);
    addColor(LINGMO_ORANGE);
    addColor(LINGMO_PURPLE);
    addColor(LINGMO_YELLOW);
    addColor(LINGMO_GREY);
    addColor(LINGMO_PINK);

}

void SelectColor::showEvent(QShowEvent *event)
{
    CustomColorPanel::showEvent(event);
}

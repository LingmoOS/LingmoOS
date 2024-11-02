/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "bluetoothmainloadingwindow.h"

BluetoothMainLoadingWindow::BluetoothMainLoadingWindow()
{
    InitAdapterLoadingWidget();
}

BluetoothMainLoadingWindow::~BluetoothMainLoadingWindow()
{

}

void BluetoothMainLoadingWindow::InitAdapterLoadingWidget()
{
    QVBoxLayout *loadingWidgetLayout = new QVBoxLayout(this);
    loadingWidgetIcon = new LoadingLabel(this);

    this->setObjectName("adapterLoadingWidget");
    loadingWidgetLayout->setSpacing(10);
    loadingWidgetLayout->setMargin(0);
    loadingWidgetLayout->setContentsMargins(0,0,0,0);

    loadingWidgetIcon->setFixedSize(16,16);
    loadingWidgetIcon->setTimerStart();
    loadingWidgetLayout->addStretch(10);

    loadingWidgetLayout->addWidget(loadingWidgetIcon,1,Qt::AlignCenter);
    loadingWidgetLayout->addStretch(10);

}

void BluetoothMainLoadingWindow::DisplayLoadingWindow()
{
    if(loadingWidgetIcon)
        loadingWidgetIcon->setTimerStart();
}

void BluetoothMainLoadingWindow::HiddenLoadingWindow()
{
    if(loadingWidgetIcon)
        loadingWidgetIcon->setTimerStop();
}


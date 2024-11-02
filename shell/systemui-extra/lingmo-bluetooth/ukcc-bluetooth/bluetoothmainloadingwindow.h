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
#ifndef BLUETOOTHMAINLOADINGWINDOW_H
#define BLUETOOTHMAINLOADINGWINDOW_H

#include <QWidget>
#include <QObject>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "loadinglabel.h"

class BluetoothMainLoadingWindow : public QWidget
{
    Q_OBJECT
public:
    BluetoothMainLoadingWindow();
    ~BluetoothMainLoadingWindow();

    void DisplayLoadingWindow();
    void HiddenLoadingWindow();
private:
    void InitAdapterLoadingWidget();

private:
    LoadingLabel      *loadingWidgetIcon = nullptr;

};

#endif // BLUETOOTHMAINLOADINGWINDOW_H

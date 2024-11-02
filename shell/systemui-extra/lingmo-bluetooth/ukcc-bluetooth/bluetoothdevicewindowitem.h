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
#ifndef BLUETOOTHDEVICEWINDOWITEM_H
#define BLUETOOTHDEVICEWINDOWITEM_H

#include <QFrame>
#include <QObject>
#include <QWidget>

#include <QVBoxLayout>

#include "bluetoothdeviceitem.h"
class bluetoothdeviceitem;
class bluetoothdevicewindowitem : public QFrame
{
    Q_OBJECT
public:
    bluetoothdevicewindowitem(QString dev_address,bool show_line = false, QWidget *parent = nullptr);
    ~bluetoothdevicewindowitem();

    void setLineFrameHidden(bool hidden);

signals:

    void devFuncOptSignals();
    void devPairedSuccess(QString);
    void devConnectedChanged(bool);
    void devRssiChanged(qint64);
    void bluetoothDeviceItemRemove(QString);
private:
    bool m_show_line = false ;
    QString m_dev_address ;
    bluetoothdeviceitem * m_devBtn;
    QFrame *line_frame = nullptr;

private:
    void Init();



};

#endif // BLUETOOTHDEVICEWINDOWITEM_H

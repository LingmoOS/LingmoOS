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

#ifndef BLUETOOTHSETTINGLABEL_H
#define BLUETOOTHSETTINGLABEL_H

#include <QLabel>
#include <QGSettings>
#include <QPainter>
#include <QPaintDevice>
#include "config/config.h"

#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"

class BluetoothSettingLabel : public QLabel
{
    Q_OBJECT
public:
    BluetoothSettingLabel(QWidget *parent);
    ~BluetoothSettingLabel();

    enum Status{
        Hover = 0,
        Normal,
        Click
    };
    Q_ENUM(Status)

    void setFocusStyle(bool val);

private:

    QGSettings *StyleSettings = nullptr;
    QGSettings *transparency_gsettings = nullptr;
    double tran =1;
    Status _MStatus;
    bool focused = false;

    void drawBackground(QPainter &painter);
    QColor getPainterBrushColor();

signals:
    void hideFeature();
    void clickSignal();

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *event);
};

#endif // BLUETOOTHSETTINGLABEL_H

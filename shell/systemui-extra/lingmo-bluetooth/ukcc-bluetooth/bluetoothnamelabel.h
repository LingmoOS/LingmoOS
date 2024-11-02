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

#ifndef BLUETOOTHNAMELABEL_H
#define BLUETOOTHNAMELABEL_H

#include <QIcon>
#include <QFont>
#include <QLabel>
#include <QDebug>
#include <QObject>
#include <QWidget>
#include <QString>
#include <QPalette>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFontMetrics>
#include <lingmo-log4qt.h>
#include <qapplication.h>
#include <QGSettings/QGSettings>

//#include <ukcc/widgets/imageutil.h>

#include "devrenamedialog.h"
#include "ukccbluetoothconfig.h"

#define DEVNAMELENGTH   30
#define ICON_PENCIL_W   16
#define ICON_PENCIL_H   16
#define ICON_PENCIL_W_H (ICON_PENCIL_W,ICON_PENCIL_H)

class BluetoothNameLabel : public QWidget
{
    Q_OBJECT
public:
    BluetoothNameLabel(QWidget *parent = nullptr, int x = 200,int y = 40);
    ~BluetoothNameLabel();
    void set_dev_name(const QString &dev_name);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);

    void leaveEvent(QEvent *event);
    void enterEvent(QEvent *event);

    void resizeEvent(QResizeEvent *event);
signals:
    void sendAdapterName(const QString &value);

public slots:
    void settings_changed(const QString &key);

private:
    QGSettings *settings;
    bool style_flag = false;

    QLabel *m_label = nullptr;
    QLabel *icon_pencil=nullptr;

    QString device_name;
    int font_width;

    QMessageBox *messagebox = nullptr;
    QHBoxLayout *hLayout = nullptr;

    DevRenameDialog *renameDialog = nullptr;
    void showDevRenameDialog();
    void setMyNameLabelText(QString);
};

#endif // BLUETOOTHNAMELABEL_H

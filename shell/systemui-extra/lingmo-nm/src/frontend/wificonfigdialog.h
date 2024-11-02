/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef WIFICONFIGDIALOG_H
#define WIFICONFIGDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QPoint>
#include <QGSettings/QGSettings>

namespace Ui {
class WiFiConfigDialog;
}

class WiFiConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WiFiConfigDialog(QWidget *parent = nullptr);
    ~WiFiConfigDialog();

    void initTransparentState();
    double getTransparentData();
    void toConfigWifi();
    void setEnableOfBtn();

protected:
    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void on_btnCancel_clicked();

    void on_btnOk_clicked();

    void on_leWifiId_textEdited(const QString &arg1);

    void on_leWiFiName_textEdited(const QString &arg1);

    void on_leWifiPassword_textEdited(const QString &arg1);

    void on_checkBoxPwd_clicked();

//    void on_checkBoxPwd_released();

private:
    Ui::WiFiConfigDialog *ui;
    QGSettings *m_transparency_gsettings = nullptr;
    QString     checkBoxQss = "QCheckBox {border:none;background:transparent;}"
                              "QCheckBox::indicator {width: 18px; height: 9px;}"
                              "QCheckBox::indicator:checked {image: url(:/res/h/show-pwd.png);}"
                              "QCheckBox::indicator:unchecked {image: url(:/res/h/hide-pwd.png);}";
};

#endif // WIFICONFIGDIALOG_H

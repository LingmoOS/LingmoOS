/*
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "login-remote-filesystem.h"
#include "ui_login-remote-filesystem.h"

#include <QDebug>

LoginRemoteFilesystem::LoginRemoteFilesystem(QWidget *parent) :
    QDialog(parent), ui(new Ui::LoginRemoteFilesystem)
{
    ui->setupUi(this);
    ui->pwd_lineEdit->setEchoMode(QLineEdit::Password);

    ui->port_comboBox->setEditText("445");

//    connect(ui->type_comboBox, &QComboBox::currentTextChanged, [=](QString& tp) {
//        if ("SAMBA" == tp) {
//            ui->port_comboBox->setEditText("445");
//        } else if ("FTP" == tp) {
//            ui->port_comboBox->setEditText("21");
//        }
//    });
}

LoginRemoteFilesystem::~LoginRemoteFilesystem()
{
    disconnect();
    delete ui;
}

QString LoginRemoteFilesystem::user()
{
    return ui->user_lineEdit->text();
}

QString LoginRemoteFilesystem::password()
{
    return ui->pwd_lineEdit->text();
}

QString LoginRemoteFilesystem::domain()
{
    return ui->ip_edit->text() + ":" + ui->port_comboBox->currentText();
}

QString LoginRemoteFilesystem::uri()
{
    QString uuri = "";

    if (ui->type_comboBox->currentText() == "SAMBA") {
        uuri = "smb://" + ui->ip_edit->text() + ":" + ui->port_comboBox->currentText() + ui->file_lineEdit->text();
    } else if (ui->type_comboBox->currentText() == "FTP") {
        uuri = "ftp://" + ui->ip_edit->text() + ":" + ui->port_comboBox->currentText() + ui->file_lineEdit->text();
    }

    return uuri;
}


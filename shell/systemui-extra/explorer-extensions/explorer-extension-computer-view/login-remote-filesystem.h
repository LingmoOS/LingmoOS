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

#ifndef LOGINREMOTEFILESYSTEM_H
#define LOGINREMOTEFILESYSTEM_H

#include <QDialog>

namespace Ui {
class LoginRemoteFilesystem;
}

class LoginRemoteFilesystem : public QDialog
{
    Q_OBJECT

public:
    explicit LoginRemoteFilesystem(QWidget *parent = nullptr);
    ~LoginRemoteFilesystem();

    QString user();
    QString password();
    QString domain();
    QString uri();

private:
    Ui::LoginRemoteFilesystem *ui;
};

#endif // LOGINREMOTEFILESYSTEM_H

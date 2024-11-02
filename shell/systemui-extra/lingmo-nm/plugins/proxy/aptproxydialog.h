/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef APTPROXYDIALOG_H
#define APTPROXYDIALOG_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QGSettings>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include "fixlabel.h"

#define APT_PROXY_SCHEMA              "org.lingmo.control-center.apt.proxy"
#define APT_PROXY_ENABLED            "enabled"
#define APT_PROXY_HOST_KEY         "host"
#define APT_PROXY_PORT_KEY         "port"

class AptProxyDialog : public QDialog
{
    Q_OBJECT
public:
    AptProxyDialog(QWidget *parent = nullptr);
    ~AptProxyDialog();

    void initUi();
private:
    QLineEdit *mHostEdit;
    QLineEdit *mPortEdit;

    QPushButton *mCancelBtn;
    QPushButton *mConfirmBtn;

private:
    void initConnect();
     void setupComponent();
};

#endif // APTPROXYDIALOG_H

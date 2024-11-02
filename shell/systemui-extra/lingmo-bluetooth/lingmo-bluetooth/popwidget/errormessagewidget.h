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

#ifndef ERRORMESSAGEWIDGET_H
#define ERRORMESSAGEWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QGSettings>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QApplication>

#include <config/config.h>
#include <config/xatom-helper.h>
#include "beforeturnoffhintwidget.h"

class ErrorMessageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ErrorMessageWidget(QWidget *parent = nullptr);
    ~ErrorMessageWidget();
    void ErrCodeAnalysis(QString dev, int errCode);
    void showCloseInfoDialog();

public slots:
    void closeMessageWidget(void);

private:
    QGSettings *gsettings;
    QLabel *warn_icon;
    QLabel *warn_title;
    QLabel *warn_txt;
    QPushButton *ok_btn;
    QMessageBox *mbox = nullptr;
    bool msgBoxShowed = false;
    BeforeTurnOffHintWidget *closeInfoDialog;

    void GSettingsChanges(QString key);
    void TimeOutLayoutInit(QString dev);
    void showMessageBox(QString title, QString text, QString information);
signals:
    void messageBoxShowed(bool);
    void closeInfoDialogConfirmed(bool);
};

#endif // ERRORMESSAGEWIDGET_H

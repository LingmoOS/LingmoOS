/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 */


#ifndef LINGMOWEBVIEWDIALOG_H
#define LINGMOWEBVIEWDIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include "lingmo/ilingmopanelplugin.h"
namespace Ui {
class LingmoUIWebviewDialog;
}
enum LingmoUIWebviewDialogStatus{ST_HIDE,ST_SHOW};

enum CalendarShowMode
{
    lunarSunday        = 0,//show lunar and first day a week is sunday
    lunarMonday        = 1,//show lunar and first day a week is monday
    solarSunday        = 2,//show solar and first day a week is sunday
    solarMonday        = 3,//show solar and first day a week is monday
    Unknown            = 0xff
};

class LingmoUIWebviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LingmoUIWebviewDialog(QWidget *parent = nullptr);
    ~LingmoUIWebviewDialog();
    void creatwebview(int _mode, int _panelSize);
    void showinfo(QString string);
Q_SIGNALS:
    void deactivated();

private:
    Ui::LingmoUIWebviewDialog *ui;
    QSize      mQsize;
protected:
//    virtual bool event(QEvent *event);
//    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    bool eventFilter(QObject *, QEvent *);
};

#endif // LINGMOWEBVIEWDIALOG_H

/*
 * Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef DLGHOTSPOTCREATE_H
#define DLGHOTSPOTCREATE_H

#include <QDialog>
#include <QMouseEvent>
#include <QDebug>

namespace Ui {
class DlgHotspotCreate;
}

class DlgHotspotCreate : public QDialog
{
    Q_OBJECT

public:
    explicit DlgHotspotCreate(QString wiFiCardName, QWidget *parent = nullptr);
    ~DlgHotspotCreate();

public Q_SLOTS:
    void changeDialog();

private Q_SLOTS:
    void on_btnCancel_clicked();

    void on_btnOk_clicked();

    void on_checkBoxPwd_stateChanged(int arg1);

    void on_leNetName_textEdited(const QString &arg1);

    void on_lePassword_textEdited(const QString &arg1);

private:
    Ui::DlgHotspotCreate *ui;

    QString wirelessCardName;

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool isPress;
    QPoint winPos;
    QPoint dragPos;

Q_SIGNALS:
    void updateHotspotList();
    void btnHotspotState();
};

#endif // DLGHOTSPOTCREATE_H

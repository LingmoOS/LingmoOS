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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef NOTEEXITWINDOW_H
#define NOTEEXITWINDOW_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QBitmap>

class Widget;
namespace Ui {
class noteExitWindow;
}

class noteExitWindow : public QDialog
{
    Q_OBJECT

public:
    explicit noteExitWindow(Widget* page, QWidget *parent=nullptr);
    ~noteExitWindow();
    void exitImmediate();

protected:
    void paintEvent(QPaintEvent*);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::noteExitWindow *ui;
    Widget* pNotebook;

signals:
    void requestSetNoteNull();
};

#endif // NOTEEXITWINDOW_H

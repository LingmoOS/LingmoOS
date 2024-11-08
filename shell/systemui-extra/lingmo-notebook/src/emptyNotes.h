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

#ifndef EMPTYNOTES_H
#define EMPTYNOTES_H

#include <QDialog>

namespace Ui {
class emptyNotes;
}

class emptyNotes : public QDialog
{
    Q_OBJECT

public:
    explicit emptyNotes(QWidget *parent = nullptr);
    ~emptyNotes();

    bool m_isDontShow;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    void paintEvent(QPaintEvent*);
    Ui::emptyNotes *ui;

signals:
    void requestEmptyNotes();

};

#endif // EMPTY_NOTEPAD_H

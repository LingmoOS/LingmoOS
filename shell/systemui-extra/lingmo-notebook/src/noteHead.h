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

#ifndef NOTEHEAD_H
#define NOTEHEAD_H

#include <QWidget>
#include <QPainter>

namespace Ui {
class noteHead;
}

class noteHead : public QWidget
{
    Q_OBJECT

public:
    explicit noteHead(QWidget *parent = nullptr);
    ~noteHead();

public:
    QColor colorWidget;

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::noteHead *ui;
};

#endif // NOTEHEAD_H

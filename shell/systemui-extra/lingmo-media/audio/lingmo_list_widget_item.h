/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#ifndef LINGMOLISTWIDGETITEM_H
#define LINGMOLISTWIDGETITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>
#include <QString>
#include <QListWidgetItem>
#include <QListWidget>
#include <QPushButton>

class LingmoUIListWidget : public QListWidget
{
    Q_OBJECT
public:
    LingmoUIListWidget(QWidget *parent = nullptr);
    ~LingmoUIListWidget();
protected:
    void paintEvent(QPaintEvent*event) {
        int i;
        for (i = 0 ;i < this->count();i++) {
            QListWidgetItem *item = this->item(i);
//            item->setTextColor(QColor(0,0,0,0));
            delete item;
        }
        QListWidget::paintEvent(event);
    }

};

class LingmoUIListWidgetItem : public QWidget
{
    Q_OBJECT

public:
    LingmoUIListWidgetItem(QWidget *parent = 0);
    ~LingmoUIListWidgetItem();

public:
    void setLabelText(QString portText,QString deviceLabel);
//    void setLabelTextIsWhite(bool selected);

    void setSelected(bool selected);

//    QString text();

    QString portName;
    QLabel * portLabel;
    QLabel * deviceLabel;

protected:
//    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *ev);

private:
    QWidget * widget;

};

#endif // LINGMOLISTWIDGETITEM_H

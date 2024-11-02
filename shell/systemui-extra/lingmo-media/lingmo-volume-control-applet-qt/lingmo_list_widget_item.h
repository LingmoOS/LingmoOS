/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
#ifndef LINGMOLISTWIDGETITEM_H
#define LINGMOLISTWIDGETITEM_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <qvariant.h>
#include "ukmedia_custom_class.h"

class LingmoUIListWidgetItem : public QWidget
{
    Q_OBJECT
public:
    LingmoUIListWidgetItem(QWidget *parent = 0);
    ~LingmoUIListWidgetItem();
    void setLabelText(QString portText,QString deviceLabel);
    void setSelected(bool selected);
    void setButtonIcon(QString iconName);

    QLabel          *deviceLabel;
    FixLabel        *portLabel;
    QPushButton     *deviceButton;

private:
    QHBoxLayout     *hLayout;

protected:
    void mousePressEvent(QMouseEvent *event);

};

#endif // LINGMOLISTWIDGETITEM_H

/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "widget.h"
#include <QHBoxLayout>
#include <QColor>
#include <QList>
#include "kcolorcombobox.h"
#include <QDebug>
using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    KColorComboBox *comboBoxCircle = new KColorComboBox(this);
    comboBoxCircle->setComboType(KColorComboBox::Circle);
    connect(comboBoxCircle,&KColorComboBox::currentColorChanged,this,[=](){
        qDebug()<<"currentColorChanged";
    });
    connect(comboBoxCircle,&KColorComboBox::activated,this,[=](){
        qDebug()<<"activatedChanged";
    });
    connect(comboBoxCircle,&KColorComboBox::highlighted,this,[=](){
        qDebug()<<"highlightChanged";
    });
    hLayout->addWidget(comboBoxCircle);
    comboBoxCircle->setFixedSize(36,36);
    KColorComboBox *comboBoxRect = new KColorComboBox(this);
    connect(comboBoxRect,&KColorComboBox::currentColorChanged,this,[=](){
        qDebug()<<"currentColorChanged";
    });
    connect(comboBoxRect,&KColorComboBox::activated,this,[=](){
        qDebug()<<"activatedChanged";
    });
    connect(comboBoxRect,&KColorComboBox::highlighted,this,[=](){
        qDebug()<<"highlightChanged";
    });
    comboBoxRect->setComboType(KColorComboBox::RoundedRect);
    hLayout->addWidget(comboBoxRect);
    comboBoxRect->setFixedSize(36,36);

//    1.列表插入
    QList <QColor> list;
    list.append(QColor(255,0,0));
    list.append(QColor(0,255,0));
    list.append(QColor(255,255,0));
    comboBoxCircle->setColorList(list);
//    2.逐个插入
    comboBoxRect->addColor(QColor(255,0,0));
    comboBoxRect->addColor(QColor(0,255,0));
    comboBoxRect->addColor(QColor(255,255,0));
    setGeometry(400,400,400,400);
        comboBoxCircle->setCurrentIndex(1);
}

Widget::~Widget()
{
}


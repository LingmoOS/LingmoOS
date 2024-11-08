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
#include <QPushButton>
#include <QVBoxLayout>
#include "kmessagebox.h"

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;

    QPushButton *btn_1 = new QPushButton(this);
    btn_1->setText("TestDemoOne");
    QObject::connect(btn_1,&QPushButton::clicked,this,[=](){
        KMessageBox w;
        w.setCustomIcon(QIcon::fromTheme("dialog-error"));
        w.setText("打印测试失败，建议修改驱动后重试");

        QPushButton *btn1 = new QPushButton("修改驱动");
        QPushButton *btn2 = new QPushButton("取消");
        w.addButton(btn1,KMessageBox::YesRole);
        w.addButton(btn2,KMessageBox::NoRole);

        w.exec();
    });

    QPushButton *btn_2 = new QPushButton(this);
    btn_2->setText("TestDemoTwo");
    QObject::connect(btn_2,&QPushButton::clicked,this,[=](){
        KMessageBox w;
        w.setCustomIcon(QIcon::fromTheme("lingmo-dialog-success"));
        w.setText("添加成功，是否打印测试页？");

        QPushButton *btn1 = new QPushButton("打印测试页");
        QPushButton *btn2 = new QPushButton("查看设备");
        w.addButton(btn1,KMessageBox::YesRole);
        w.addButton(btn2,KMessageBox::NoRole);

        w.exec();
    });

    layout->addWidget(btn_1);
    layout->addWidget(btn_2);

    this->setLayout(layout);
    this->setFixedSize(300,200);
}

Widget::~Widget()
{
}


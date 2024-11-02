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
 * Authors: Xiangwei Liu <liuxiangwei@kylinos.cn>
 *
 */


#include "widget.h"
#include "kpushbutton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

using namespace kdk;


Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout1 = new QHBoxLayout();  /*半透明字体按钮布局，包括按钮1、2、3*/

    KPushButton *btn1 = new KPushButton(this); /*不可用透明按钮*/
    btn1->setText("button1");
    btn1->setTranslucent(true);
    btn1->setEnabled(false);

    KPushButton *btn2 = new KPushButton(this); /*可用透明按钮*/
    btn2->setText("button2");
    btn2->setTranslucent(true);
    btn2->setBackgroundColor(Qt::blue);

    KPushButton *btn3 = new KPushButton(this); /*圆形透明按钮,如果需要设置为正圆需要自己设置大小，默认跟随layout*/
    btn3->setText("button3");
    btn3->setTranslucent(true);
    btn3->setButtonType(KPushButton::CircleType);

    layout1->addWidget(btn1);
    layout1->addWidget(btn2);
    layout1->addWidget(btn3);

    QHBoxLayout *layout2 = new QHBoxLayout();  /*半透明图标按钮，包括按钮4、5*/

    KPushButton *btn4 = new KPushButton(this);
    btn4->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    btn4->setTranslucent(true);
    btn4->setFixedSize(32,32);
    btn4->setIconSize(QSize(24,24));
    btn4->setBackgroundColor(Qt::blue);
    KPushButton *btn5 = new KPushButton(this);
    btn5->setIcon(QIcon::fromTheme("go-next-symbolic"));
    btn5->setTranslucent(true);
    btn5->setFixedSize(32,32);
    btn5->setIconSize(QSize(24,24));

    layout2->addWidget(btn4);
    layout2->addWidget(btn5);


    QHBoxLayout *layout3 = new QHBoxLayout();  /*正常带图标不透明按钮，包括6、7*/
    KPushButton *btn6 = new KPushButton(this);
    btn6->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    btn6->setFixedSize(32,32);
    btn6->setIconSize(QSize(24,24));
    btn6->setIconHighlight(true);
    btn6->setBackgroundColor(Qt::transparent);

    KPushButton *btn7 = new KPushButton(this);
    btn7->setIcon(QIcon::fromTheme("go-next-symbolic"));
    btn7->setFixedSize(32,32);
    btn7->setIconSize(QSize(24,24));
    btn7->setIconHighlight(true);
    btn7->setBackgroundColor(Qt::transparent);

    layout3->addWidget(btn6);
    layout3->addWidget(btn7);

    QHBoxLayout *layout4 = new QHBoxLayout();

    KPushButton *btn8 = new KPushButton(this); /*设置文字且设置圆角的不透明按钮*/
    btn8->setText("button8");
    btn8->setBorderRadius(10);

    KPushButton *btn9 = new KPushButton(this);   /*设置图标且设置圆角的不透明按钮*/
    btn9->setIcon(QIcon::fromTheme("list-add-symbolic"));
    btn9->setIconColor(Qt::white);
    btn9->setBackgroundColorHighlight(true);    /*设置按钮背景色跟随系统高亮色*/
    //btn9->setBackgroundColor(Qt::red);          /*用户设置自定义背景色（与跟随系统高亮存在先后关系，后设置的生效）*/

    btn9->setFixedSize(40,40);
    btn9->setBorderRadius(20);
    btn9->setIconSize(QSize(24,24));

    layout4->addWidget(btn8);
    layout4->addWidget(btn9);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(layout1);
    layout->addLayout(layout2);
    layout->addLayout(layout3);
    layout->addLayout(layout4);

    this->setLayout(layout);
}

Widget::~Widget()
{
}

void Widget::onButtonClicked()
{
    qDebug() << "clicked" ;
}


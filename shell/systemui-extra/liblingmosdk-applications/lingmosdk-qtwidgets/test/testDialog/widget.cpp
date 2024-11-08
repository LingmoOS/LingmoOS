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

#include <QBoxLayout>
#include <QPushButton>

#include "kdialog.h"
#include "kuninstalldialog.h"
#include "kaboutdialog.h"

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    QPushButton*pBtn1 =  new QPushButton("对话框",this);
    QPushButton*pBtn2 = new QPushButton("卸载对话框",this);
    QPushButton*pBtn3 = new QPushButton("关于对话框",this);
    KDialog*dialog = new KDialog();
    //设置标题
    dialog->setWindowTitle("对话框");
    //设置icon
    dialog->setWindowIcon("lingmo-music");
    //显示最小化和最大化按钮
    dialog->maximumButton()->show();
    dialog->minimumButton()->show();

    KUninstallDialog *uninstallDialog = new KUninstallDialog("browser360-cn-stable","104",this);
    KAboutDialog *aboutDialog = new KAboutDialog(this,QIcon::fromTheme("lingmo-music"),"灵墨音乐",tr("版本：2020.1.0"));
    aboutDialog->setBodyText("关于对话框，包含的主要内容有：应用图标，应用名称，版本号，团队邮箱以及具体的应用描述，注意，默认应用描述是不显示的。");
    aboutDialog->setBodyTextVisiable(true);
    vLayout->addStretch();
    vLayout->addWidget(pBtn1);
    vLayout->addWidget(pBtn2);
    vLayout->addWidget(pBtn3);
    vLayout->addStretch();
    connect(pBtn1,&QPushButton::clicked,this,[=](){dialog->show();});
    connect(pBtn2,&QPushButton::clicked,this,[=](){uninstallDialog->show();});
    connect(pBtn3,&QPushButton::clicked,this,[=](){aboutDialog->show();});
//    QHBoxLayout* tmpLayout = dynamic_cast<QHBoxLayout*>(windowButtonBar()->layout());
//    QPushButton* btn = new QPushButton("btn",this);
//    tmpLayout->insertWidget(1,btn);
    QHBoxLayout *hLayout = new QHBoxLayout(dialog);
    QPushButton*pBtn5 =  new QPushButton("对话框",dialog);
    hLayout->addWidget(pBtn5);
    dialog->mainWidget()->setLayout(hLayout);   //获取主内容区，通过setLayout()添加内容
    dialog->menuButton()->show();//获取下拉菜单按钮，默认是隐藏的,不显示
    //dialog->closeButton()->show();//获取关闭按钮。
    baseBar()->setLayout(vLayout);
}

Widget::~Widget()
{
}


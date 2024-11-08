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
#include "kprogressbar.h"
#include <QVBoxLayout>
#include <QPushButton>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    KProgressDialog *progressDialog = new KProgressDialog(tr("运行中"),tr("取消"),0,100,this);
    progressDialog->setSubContent("正在下载...");   //设置次级内容
    progressDialog->setSuffix("MB");      //设置detail的后缀
    progressDialog->setWindowTitle("进度对话框");   //设置窗口标题
    progressDialog->setWindowIcon("lingmo-music");  //设置窗口图标
    progressDialog->setValue(98);    //设置当前进度值
    progressDialog->setShowDetail(true);        //设置是否显示详细信息
    progressDialog->setAutoReset(false);        //设置进度条是否重置


    QPushButton *pBtn = new QPushButton("显示",this);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(pBtn);
    baseBar()->setLayout(vLayout);

    connect(pBtn,&QPushButton::clicked,this,[=](){progressDialog->show();});
}

Widget::~Widget()
{
}


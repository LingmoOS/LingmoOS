/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#include "screentype.h"
#include <QDebug>
ScreenType::ScreenType(QWidget *parent) :  QWidget(parent)
{
    vbox = new QVBoxLayout(this);
    MP4_TYPE = new QRadioButton();
    MP4_TYPE->setText("MP4");
    GIF_TYPE = new QRadioButton();
    GIF_TYPE->setText("GIF");
    MKV_TYPE = new QRadioButton();
    MKV_TYPE->setText("mkv");
    WEBM_TYPE = new QRadioButton();
    WEBM_TYPE->setText("WEBM");
    OGG_TYPE = new QRadioButton();
    OGG_TYPE->setText("ogg");
    vbox->addWidget(MP4_TYPE,0);
    MP4_TYPE->setChecked(true);
    vbox->addWidget(GIF_TYPE,1);
    vbox->addWidget(MKV_TYPE,2);
    vbox->addWidget(WEBM_TYPE,3);
    vbox->addWidget(OGG_TYPE,4);
    connect(MP4_TYPE, SIGNAL(clicked()), this, SLOT(onRadioClickType()));
    connect(GIF_TYPE, SIGNAL(clicked()), this, SLOT(onRadioClickType()));
    connect(MKV_TYPE, SIGNAL(clicked()), this, SLOT(onRadioClickType()));
    connect(WEBM_TYPE, SIGNAL(clicked()), this, SLOT(onRadioClickType()));
    connect(OGG_TYPE, SIGNAL(clicked()), this, SLOT(onRadioClickType()));
}

void ScreenType::onRadioClickType()
{
    if(MP4_TYPE->isChecked())
    {
       qDebug()<<"MP4_TYPE";
       type = "MP4";
    }
    else if(GIF_TYPE->isChecked()) {
        qDebug()<<"GIF_TYPE";
        type = "GIF";
    }
    else if(MKV_TYPE->isChecked())
    {
        qDebug()<<"MKV_TYPE";
        type = "MKV";
    }
    else if(WEBM_TYPE->isChecked())
    {
        qDebug()<<"WEBM_TYPE";
        type = "WEBM";
    }
    else if(OGG_TYPE->isChecked())
    {
        qDebug()<<"OGG_TYPE";
        type = "ogg";
    }
    else
    {
        qDebug()<<"ERROR";
    }
   emit  Type_change(type);
}

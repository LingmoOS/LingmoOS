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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "loadingdiv.h"
#include <QIcon>

#define FRAMESPEED 60 //帧与帧之间的间隔时间(ms)
#define ALLTIME 40*1000 //等待动画持续总时间

//加载动画控件'loadingGif'
LoadingDiv::LoadingDiv(QWidget *parent) : QWidget(parent)
{
    this->resize(480, 538);

    this->loadingGif = new QLabel(this);
    this->loadingGif->resize(96, 96);
    this->loadingGif->move(this->width()/2 - 96/2 + 41/2 - 40, this->height()/2 + 20);
    this->loadingGif->show();

    this->switchTimer = new QTimer(this); //QTimer对象，控制等待动画播放
    connect(switchTimer, SIGNAL(timeout()), this, SLOT(switchAnimStep()));

    this->raise();
    this->hide();
}

//加载动画控件'loadingGif'
void LoadingDiv::switchAnimStep()
{
    //另外一种加载方法
    QString qpmQss = ":/res/s/conning-b/";
    qpmQss.append(QString::number(this->currentPage));
    qpmQss.append(".png");
    loadingGif->setPixmap(QPixmap(qpmQss));
    loadingGif->setProperty("useIconHighlightEffect", true);
    loadingGif->setProperty("iconHighlightEffectMode", true);

    this->currentPage --;

    if (this->currentPage < 1) {
        this->currentPage = 12; //循环播放
    }
    this->countCurrentTime += FRAMESPEED;
    if (this->countCurrentTime >= ALLTIME) {
        Q_EMIT this->toStopLoading(); //发出信号停止主界面和托盘区的等待动画
    }
}

//开始播放动画
void LoadingDiv::startLoading()
{
    this->currentPage = 12;
    this->countCurrentTime = 0;
    this->switchTimer->start(FRAMESPEED);
    this->show();
}

//结束播放动画
void LoadingDiv::stopLoading()
{
    this->switchTimer->stop();
    this->hide();
}

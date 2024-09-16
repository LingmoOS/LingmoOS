// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "moduleinterface.h"

ModuleInterface::ModuleInterface(QWidget *parent)
    : DFrame(parent)
    , m_model(Model::Instance())
    , m_worker(Worker::Instance())
    , m_selectBtn(new IconButton(this))
    , m_updateSelectBtnTimer(new QTimer(this))
{
    m_updateSelectBtnTimer->setInterval(0);
    m_updateSelectBtnTimer->setSingleShot(true);
    setLineWidth(0);

    connect(m_updateSelectBtnTimer, &QTimer::timeout, this, &ModuleInterface::updateSelectBtnPos);
}

/*******************************************************************************
 1. @函数:    setIconType
 2. @作者:
 3. @日期:    2020-12-10
 4. @说明:    根据图标的大小，更新图标，首次启动的图标大，日常启动的图标小
*******************************************************************************/
void ModuleInterface::setIconType(Model::IconType type)
{
    switch (type) {
    case Model::Big:
        updateBigIcon();
        break;
    case Model::Small:
        updateSmallIcon();
        break;
    }
}

//调整大小事件，大小变化启动更新选择按钮定时器
void ModuleInterface::resizeEvent(QResizeEvent *event)
{
    DFrame::resizeEvent(event);
    m_updateSelectBtnTimer->start();
}

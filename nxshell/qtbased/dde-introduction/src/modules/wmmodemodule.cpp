// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wmmodemodule.h"

WMModeModule::WMModeModule(QWidget *parent)
    : ModuleInterface(parent)
    , m_layout(new QHBoxLayout(this))
    , m_efficientWidget(new BaseWidget(this))
    , m_fashionWidget(new BaseWidget(this))
{
    m_first = true;
    m_efficientWidget->setTitle(tr("Normal Mode"));
    m_fashionWidget->setTitle(tr("Effect Mode"));

    connect(m_model, &Model::wmTypeChanged, this, &WMModeModule::onWMModeChanged);
    connect(m_fashionWidget, &BaseWidget::clicked, this, &WMModeModule::onSetModeEfficient);
    connect(m_efficientWidget, &BaseWidget::clicked, this, &WMModeModule::onSetModeNormal);
    connect(m_efficientWidget, &BaseWidget::sizeChanged, this, &WMModeModule::updateSelectBtnPos);

    //特效模式和普通模式切换响应
    onWMModeChanged(m_model->wmType());

    m_layout->setContentsMargins(0, 30, 0, 65);

    m_layout->addWidget(m_fashionWidget);
    m_layout->addWidget(m_efficientWidget);

    setLayout(m_layout);

    updateSmallIcon();
}

//首次启动初始化图片
void WMModeModule::updateBigIcon()
{
    m_efficientWidget->setBigPixmap(":/resources/2d_big@3x.png");
    m_fashionWidget->setBigPixmap(":/resources/3d_big@3x.png");
    m_layout->setContentsMargins(10, 70, 10, 35);
}

//日常启动初始化图片
void WMModeModule::updateSmallIcon()
{
    m_efficientWidget->setSmallPixmap(":/resources/2d_small@3x.png");
    m_fashionWidget->setSmallPixmap(":/resources/3d_small@3x.png");
}

//更新选择按钮位置
void WMModeModule::updateSelectBtnPos()
{
    onWMModeChanged(m_model->wmType());
}

/*******************************************************************************
 1. @函数:    onWMModeChanged
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    特效模式和普通模式切换响应槽函数
*******************************************************************************/
void WMModeModule::onWMModeChanged(Model::WMType type)
{
    switch (type) {
        case Model::WM_2D:
            m_efficientWidget->setChecked(true);
            m_fashionWidget->setChecked(false);
            break;
        case Model::WM_3D:
            m_fashionWidget->setChecked(true);
            m_efficientWidget->setChecked(false);
            break;
        default:
            break;
    }
    update();
}

//设置首次启动标志位
void WMModeModule::setFirst(bool first)
{
    m_first = first;
}

//键盘按键事件左右切换特效和普通模式
void WMModeModule::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        if (m_model->wmType() == Model::WM_2D)
            m_worker->setWMMode(Model::WM_3D);
    } else if (event->key() == Qt::Key_Right) {
        if (m_model->wmType() == Model::WM_3D)
            m_worker->setWMMode(Model::WM_2D);
    }
}

//通过绘制函数，更新选择按钮的位置
void WMModeModule::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPoint point;

    int x = m_efficientWidget->m_borderWidget->geometry().x() +
            m_fashionWidget->m_borderWidget->geometry().width();
    int y = m_efficientWidget->m_borderWidget->geometry().y();

    if (m_first) {
        if (m_model->wmType() == Model::WM_2D) {
            point.setX(x * 2 + 2);
            point.setY(y + 63);
        } else if (m_model->wmType() == Model::WM_3D) {
            point.setX(x - 2);
            point.setY(y + 63);
        }
    } else {
        if (m_model->wmType() == Model::WM_2D) {
            point.setX(x * 2 + 20);
            point.setY(y + 24);
        } else if (m_model->wmType() == Model::WM_3D) {
            point.setX(x + 2);
            point.setY(y + 24);
        }
    }

    m_selectBtn->move(point);
    m_selectBtn->raise();
}

void WMModeModule::onSetModeNormal() {
    m_worker->setWMMode(Model::WM_2D);
}
void WMModeModule::onSetModeEfficient() {
    m_worker->setWMMode(Model::WM_3D);
}


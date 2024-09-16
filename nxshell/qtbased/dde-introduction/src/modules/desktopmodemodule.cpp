// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopmodemodule.h"

//桌面模式类
DesktopModeModule::DesktopModeModule(QWidget *parent)
    : ModuleInterface(parent)
    , m_layout(new QHBoxLayout(this))
    , m_efficientWidget(new BaseWidget(this))
    , m_fashionWidget(new BaseWidget(this))
{
    isfirst = true;
    m_efficientWidget->setTitle(tr("Efficient Mode"));
    m_fashionWidget->setTitle(tr("Fashion Mode"));

    connect(m_model, &Model::desktopModeChanged, this, &DesktopModeModule::onDesktopTypeChanged);
    //时尚模式点击响应
    connect(m_fashionWidget, &BaseWidget::clicked, this,&DesktopModeModule::onSetModeFashion);
    //高效模式点击响应
    connect(m_efficientWidget, &BaseWidget::clicked, this,&DesktopModeModule::onSetModeEfficient);
    //高效模式大小变化响应
    connect(m_efficientWidget, &BaseWidget::sizeChanged, this,
            &DesktopModeModule::updateSelectBtnPos);

    m_layout->setMargin(0);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(0, 30, 0, 65);

    m_layout->addStretch();
    m_layout->addWidget(m_fashionWidget);
    m_layout->addWidget(m_efficientWidget);
    m_layout->addStretch();

    setLayout(m_layout);

    updateSmallIcon();
}

//修改选中框状态
void DesktopModeModule::onDesktopTypeChanged(Model::DesktopMode mode)
{
    switch (mode) {
        case Model::EfficientMode:
            m_efficientWidget->setChecked(true);
            m_fashionWidget->setChecked(false);
            break;
        case Model::FashionMode:
            m_fashionWidget->setChecked(true);
            m_efficientWidget->setChecked(false);
            break;
    }
    update();
}

//首次启动初始化图片
void DesktopModeModule::updateBigIcon()
{
    m_efficientWidget->setBigPixmap(":/resources/effective_mode_big@3x.png");
    m_fashionWidget->setBigPixmap(":/resources/fashion_mode_big@3x.png");
    m_layout->setContentsMargins(10, 70, 10, 35);
    m_size = QSize(330, 210);
}

//日常启动初始化图片
void DesktopModeModule::updateSmallIcon()
{
    QPixmap pixmapeff(":/resources/effective_mode_small@3x.png");
    QPixmap pixmapfash(":/resources/fashion_mode_small@3x.png");
    m_efficientWidget->setSmallPixmap(pixmapeff);
    m_fashionWidget->setSmallPixmap(pixmapfash);
    m_size = QSize(250, 160);
}

void DesktopModeModule::updateSelectBtnPos()
{
    onDesktopTypeChanged(m_model->desktopMode());
}

void DesktopModeModule::setFirst(bool first)
{
    isfirst = first;
}

//功能??? zyf也不知道
void DesktopModeModule::updateInterface(float f)
{
    m_efficientWidget->updateInterface(f);
    m_fashionWidget->updateInterface(f);

    /*switch (m_model->desktopMode()) {
    case Model::EfficientMode:
        m_selectBtn->move(m_efficientWidget->mapTo(this, m_efficientWidget->rect().topRight()) - p);
        break;
    case Model::FashionMode:
        m_selectBtn->move(m_fashionWidget->mapTo(this, m_fashionWidget->rect().topRight()) - p);
        break;
    default:
        break;
    }*/

    update();
}

//键盘按键事件左右切换高效和时尚模式
void DesktopModeModule::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        if (m_model->desktopMode() == Model::EfficientMode)//代码不规范
            m_worker->setDesktopMode(Model::FashionMode);
    } else if (event->key() == Qt::Key_Right) {
        if (m_model->desktopMode() == Model::FashionMode)
            m_worker->setDesktopMode(Model::EfficientMode);
    }
}

//更新选择按钮的位置
void DesktopModeModule::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPoint point;

    int x = m_efficientWidget->m_borderWidget->geometry().x() +
            m_fashionWidget->m_borderWidget->geometry().width();
    int y = m_efficientWidget->m_borderWidget->geometry().y();

    if (isfirst) {
        if (m_model->desktopMode() == Model::EfficientMode) {
            point.setX(x * 2 + 15);
            point.setY(y + 63);
        } else if (m_model->desktopMode() == Model::FashionMode) {
            point.setX(x - 2);
            point.setY(y + 63);
        }
    } else {
        if (m_model->desktopMode() == Model::EfficientMode) {
            point.setX(x * 2 + 21);
            point.setY(y + 24);
        } else if (m_model->desktopMode() == Model::FashionMode) {
            point.setX(x + 2);
            point.setY(y + 24);
        }
    }

    m_selectBtn->move(point);
    m_selectBtn->raise();
}

void DesktopModeModule::onSetModeFashion() {
    m_worker->setDesktopMode(Model::FashionMode);
}
void DesktopModeModule::onSetModeEfficient() {
    m_worker->setDesktopMode(Model::EfficientMode);
}

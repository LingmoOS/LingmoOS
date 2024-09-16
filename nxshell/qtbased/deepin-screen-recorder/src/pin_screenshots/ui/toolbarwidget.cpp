// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbarwidget.h"
#include "accessibility/acTextDefine.h"
#include "utils.h"

#include <QActionGroup>
#include <QMouseEvent>
#include <DFontSizeManager>

#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOLBAR_WIDGET_SIZE = QSize(210, 56);

ToolBarWidget::ToolBarWidget(DWidget *parent): DBlurEffectWidget(parent)
{
    if (Utils::isWaylandMode) {
        setWindowFlags(Qt::Sheet | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    } else {
        //v23上ToolTip会导致工具栏的圆角失效
        setWindowFlags(Qt::FramelessWindowHint /*| Qt::WindowStaysOnTopHint*/ | Qt::ToolTip);
    }

    this->setRadius(30);
    this->setBlurEnabled(true);
    this->setMode(DBlurEffectWidget::GaussianBlur);
    this->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    // 初始化主题样式
    if (DGuiApplicationHelper::instance()->themeType() == THEMETYPE) {
        this->setMaskColor(QColor(255, 255, 255, 76));
    } else {
        this->setMaskColor(QColor(0, 0, 0, 76));
    }

    initToolBarWidget(); // 初始化按钮菜单
    // 工具栏样式跟随系统样式改变
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ToolBarWidget::onThemeTypeChange);
}

// 获取保存信息
QPair<int, int> ToolBarWidget::getSaveInfo()
{
    return m_subTool->getSaveInfo();
}

void ToolBarWidget::showAt(QPoint pos, bool isfirstTime)
{
    this->move(pos);
    if (this->isHidden())
        this->show();
    if (isfirstTime) {
        this->hide();
    }
}

// 选项按钮被点击
void ToolBarWidget::onOptionButtonClicked()
{
    m_subTool->onOptionButtonClicked();
}

// 主题变化
void ToolBarWidget::onThemeTypeChange(DGuiApplicationHelper::ColorType themeType)
{
    if (themeType == THEMETYPE) {
        this->setMaskColor(QColor(255, 255, 255, 76));
    } else {
        this->setMaskColor(QColor(0, 0, 0, 76));
    }
}

void ToolBarWidget::initToolBarWidget()
{
//    this->setFixedSize(MIN_TOOLBAR_WIDGET_SIZE);
    this->setMinimumSize(MIN_TOOLBAR_WIDGET_SIZE);
    m_subTool = new SubToolWidget(this);
    connect(m_subTool, SIGNAL(signalOcrButtonClicked()), this, SIGNAL(sendOcrButtonClicked()));

    m_mainTool = new MainToolWidget(this);
    connect(m_mainTool, SIGNAL(signalCloseButtonClicked()), this, SIGNAL(sendCloseButtonClicked()));
    connect(m_mainTool, SIGNAL(signalSaveButtonClicked()), this, SIGNAL(sendSaveButtonClicked()));

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 0, 10, 0);
    hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    hLayout->addWidget(m_mainTool, 0,  Qt::AlignCenter);
    setLayout(hLayout);
}

//重写鼠标移动事件：解决工具栏可以被拖动的问题
void ToolBarWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //qDebug() << event->button() << event->x() << event->y();
    //QWidget::mouseMoveEvent(event);
}

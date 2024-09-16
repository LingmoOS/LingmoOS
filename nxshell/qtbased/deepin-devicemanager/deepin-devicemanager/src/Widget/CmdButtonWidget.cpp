// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "CmdButtonWidget.h"
#include "DetailTreeView.h"

// Qt库文件
#include <QHBoxLayout>
#include <QPainter>
#include <QLoggingCategory>
#include <QPainterPath>

// Dtk头文件
#include <DFontSizeManager>
#include <DApplication>
#include <DApplicationHelper>

CmdButtonWidget::CmdButtonWidget(DetailTreeView *parent)
    : DWidget(parent)
    , mp_cmdButton(new DCommandLinkButton(tr("More"), this))
{
    // 初始化UI界面
    initUI();

    // 连接槽函数
    connect(mp_cmdButton, &DCommandLinkButton::clicked, this, &CmdButtonWidget::cmdButtonClicked);
}

void CmdButtonWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);

    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    this->rect().setWidth(808);
    QRect rect  = this->rect();
    rect.setWidth(808);
    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    int width = 1;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRect(rect);

    QRect rectIn = QRect(rect.x(), rect.y() + width, rect.width(), rect.height() - width);
    paintPathIn.addRect(rectIn);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::FrameShadowBorder));
    painter.fillPath(paintPath, bgBrush);


    painter.restore();
}

void CmdButtonWidget::initUI()
{
    // 设置字号
    DFontSizeManager::instance()->bind(mp_cmdButton, DFontSizeManager::T8);

    // 将mp_CommandBtn放到布局中，居中
    QHBoxLayout *pHBoxLayout = new QHBoxLayout();
    pHBoxLayout->addStretch();
    pHBoxLayout->addWidget(mp_cmdButton);
    pHBoxLayout->addStretch();

    QVBoxLayout *pVBoxLayout = new QVBoxLayout();
    pVBoxLayout->addLayout(pHBoxLayout);

    this->setLayout(pVBoxLayout);
}

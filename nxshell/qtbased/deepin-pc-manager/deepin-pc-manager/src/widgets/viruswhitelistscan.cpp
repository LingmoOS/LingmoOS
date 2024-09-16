// Copyright (C) 2021 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viruswhitelistscan.h"

#include <DFontSizeManager>
#include <DPalette>

#include <QHBoxLayout>
#include <QLabel>

VirusWhitelistScanItem::VirusWhitelistScanItem(QWidget *parent)
    : QWidget(parent)
    , m_lbText(new QLabel(this))
    , m_lbTip(new QLabel(this))
    , m_dialogBtn(new DIconButton(this))
{
    this->setContentsMargins(0, 0, 0, 0);
    // 布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    // 设置文本属性
    m_lbText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 设置提示文本属性
    m_lbTip->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_lbTip, DFontSizeManager::T9); // 字体设置
    m_lbTip->setText(tr("(Files in the whitelist will be excluded from virus scan)"));
    m_lbTip->setAlignment(Qt::AlignBottom);

    // 设置弹窗按钮
    m_dialogBtn->setAccessibleName("forwardButton");
    m_dialogBtn->setIcon(QStyle::SP_ArrowForward);
    m_dialogBtn->setFixedSize(QSize(30, 30));
    m_dialogBtn->setFlat(true);

    // 加入文本和弹窗按钮
    layout->addWidget(m_lbText);
    layout->addSpacing(10);
    layout->addWidget(m_lbTip);
    layout->addStretch(0); // 加入弹簧
    layout->addWidget(m_dialogBtn);

    // 信号绑定
    connect(m_dialogBtn, &DIconButton::clicked, this, &VirusWhitelistScanItem::pushButtonTrigger);
}

VirusWhitelistScanItem::~VirusWhitelistScanItem() { }

// 设置label显示内容
void VirusWhitelistScanItem::setText(const QString &text)
{
    m_lbText->setText(text);
}

// 按钮触发
void VirusWhitelistScanItem::pushButtonTrigger(bool status)
{
    emit buttonTrigger(status);
}

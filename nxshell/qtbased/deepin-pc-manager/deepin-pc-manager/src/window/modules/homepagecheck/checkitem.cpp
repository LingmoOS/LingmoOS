// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkitem.h"

#include "src/window/modules/common/common.h"

#include <DCommandLinkButton>
#include <DFontSizeManager>
#include <DLabel>
#include <DSpinner>

#include <QHBoxLayout>

CheckItem::CheckItem(DLabel *titlePic,
                     DLabel *title,
                     DLabel *tipPic,
                     DLabel *tip,
                     DCommandLinkButton *btn,
                     DCommandLinkButton *ignoreBtn,
                     DCommandLinkButton *notIgnoreBtn,
                     DSpinner *spinner,
                     QWidget *parent)
    : DFrame(parent)
    , m_lbTitlePic(titlePic)
    , m_lbTitle(title)
    , m_lbTipPic(tipPic)
    , m_lbTip(tip)
    , m_btn(btn)
    , m_ignoreBtn(ignoreBtn)
    , m_notIgnoreBtn(notIgnoreBtn)
    , m_spinner(spinner)
{
    // 设置背景色
    setBackgroundRole(DPalette::ItemBackground);
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    // 设置外框线宽
    setLineWidth(0);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    // 设置边界间隔
    mainLayout->setContentsMargins(20, 0, 20, 0);
    setLayout(mainLayout);

    // 添加标题图标
    mainLayout->addWidget(m_lbTitlePic);

    // 添加标题
    m_lbTitle->setFont(Utils::getFixFontSize(14));
    m_lbTitle->setFixedWidth(200);
    m_lbTitle->setWordWrap(true);
    mainLayout->addWidget(m_lbTitle);

    // 添加提示图标
    mainLayout->addWidget(m_lbTipPic);
    // 添加提示文字

    // 设置字体
    QFont ft = m_lbTip->font();
    ft.setPixelSize(12);
    ft.setWeight(QFont::Weight::Medium);
    m_lbTip->setFont(ft);
    m_lbTip->setWordWrap(true);
    mainLayout->addWidget(m_lbTip, 1);

    // 如果存在操作按钮，则添加
    if (m_btn) {
        mainLayout->addWidget(m_btn);
    }

    // 如果存在忽略按钮，则添加
    if (m_ignoreBtn) {
        mainLayout->addWidget(m_ignoreBtn);
    }

    // 如果存在取消忽略按钮，则添加
    if (m_notIgnoreBtn) {
        mainLayout->addWidget(m_notIgnoreBtn);
    }

    // 如果存在旋转按钮，则添加
    if (m_spinner) {
        mainLayout->addWidget(m_spinner);
    }
}

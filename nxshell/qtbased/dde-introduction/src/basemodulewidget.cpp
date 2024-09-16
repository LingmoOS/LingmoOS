// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basemodulewidget.h"

BaseModuleWidget::BaseModuleWidget(QWidget *content, QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
    , m_titleLbl(new DLabel(this))
    , m_describeLbl(new DLabel(this))
    , m_content(content)
{
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    m_describeLbl->setFixedSize(460, 40);
    m_describeLbl->setWordWrap(true);
    m_describeLbl->setAlignment(Qt::AlignCenter);
    m_describeLbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_describeLbl->setElideMode(Qt::TextElideMode::ElideRight);

    m_layout->addSpacing(20);
    m_layout->addWidget(m_titleLbl, 0, Qt::AlignHCenter);
    m_layout->addStretch();
    m_layout->addWidget(m_content, 0, Qt::AlignCenter);
    m_layout->addStretch();
    m_layout->addWidget(m_describeLbl, 0, Qt::AlignHCenter);
    m_layout->addSpacing(15);
    setLayout(m_layout);
}

void BaseModuleWidget::setTitle(const QString &title)
{
    QFont font;
    font.setStyleName("Bold");
    m_titleLbl->setFont(font);
    DFontSizeManager::instance()->bind(m_titleLbl, DFontSizeManager::T5);
    m_titleLbl->setText(title);
}

void BaseModuleWidget::setDescribe(const QString &describe, bool bIsFirst)
{
    QFont font;
    font.setStyleName("Normal");
    m_describeLbl->setFont(font);
    DFontSizeManager::instance()->bind(m_describeLbl, DFontSizeManager::T8);
    m_describeLbl->setText(describe);
    m_describeLbl->adjustSize();
}

void BaseModuleWidget::setContentWidgetHeight(int iHeight)
{
    if (m_content) {
        m_content->setFixedHeight(iHeight);
    }
}

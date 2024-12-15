// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadingpage.h"

#include <DLabel>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QHBoxLayout>

LoadingPage::LoadingPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
}

LoadingPage::~LoadingPage()
{

}

void LoadingPage::startLoading()
{
    m_pSpinner->start();
}

void LoadingPage::stopLoading()
{
    m_pSpinner->stop();
}

void LoadingPage::setDes(const QString &strDes)
{
    m_pTextLbl->setText(strDes);
}

void LoadingPage::initUI()
{
    // 初始化加载动画
    m_pSpinner = new DSpinner(this);
    m_pSpinner->setMinimumSize(32, 32);

    // 初始化加载提示
    m_pTextLbl = new DLabel(this);
    m_pTextLbl->setMinimumSize(293, 20);
    m_pTextLbl->setText(tr("Loading, please wait..."));
    DFontSizeManager::instance()->bind(m_pTextLbl, DFontSizeManager::T6, QFont::Medium);  // 设置字体
    DPalette pa = DApplicationHelper::instance()->palette(m_pTextLbl);
    pa.setBrush(DPalette::ButtonText, pa.color(DPalette::TextTitle));   // 设置颜色
    DApplicationHelper::instance()->setPalette(m_pTextLbl, pa);
    m_pTextLbl->setAlignment(Qt::AlignCenter);

    // 加载布局
    QHBoxLayout *pSpinnerLayout = new QHBoxLayout;
    pSpinnerLayout->addStretch();
    pSpinnerLayout->addWidget(m_pSpinner);
    pSpinnerLayout->addStretch();

    // 主布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pSpinnerLayout);
    pMainLayout->addWidget(m_pTextLbl);
    pMainLayout->addStretch();
    pMainLayout->setAlignment(Qt::AlignCenter);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

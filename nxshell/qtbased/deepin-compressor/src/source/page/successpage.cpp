// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "successpage.h"
#include "uitools.h"
#include "customwidget.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QDebug>

SuccessPage::SuccessPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

SuccessPage::~SuccessPage()
{

}

void SuccessPage::setCompressFullPath(const QString &strFullPath)
{
    m_strFullPath = strFullPath;
}

void SuccessPage::setSuccessDes(const QString &strDes)
{
    m_pSuccessLbl->setText(strDes);
}

void SuccessPage::setSuccessType(const SuccessInfo &successInfo)
{
    m_successInfoType = successInfo;
}

SuccessInfo SuccessPage::getSuccessType()
{
    return m_successInfoType;
}

void SuccessPage::setDetail(const QString &strDetail)
{
    m_pDetailLbl->setText(strDetail);
}

void SuccessPage::initUI()
{
    //成功图标
    m_pSuccessPixmapLbl = new DLabel(this);
    QPixmap m_pSuccessPixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_success_128px.svg", QSize(128, 128));
    m_pSuccessPixmapLbl->setPixmap(m_pSuccessPixmap);

    //成功文字
    m_pSuccessLbl = new DLabel(this);
    DFontSizeManager::instance()->bind(m_pSuccessLbl, DFontSizeManager::T5, QFont::DemiBold);
    m_pSuccessLbl->setForegroundRole(DPalette::ToolTipText);
    m_pSuccessLbl->setText(tr("Compression successful"));

    //失败具体原因
    m_pDetailLbl = new DLabel(this);
    m_pDetailLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pDetailLbl, DFontSizeManager::T8);

    //查看文件按钮
    m_pShowFileBtn = new CustomPushButton(this);
    m_pShowFileBtn->setMinimumSize(340, 36);
    m_pShowFileBtn->setText(tr("View"));

    //返回按钮
    m_pReturnBtn = new CustomCommandLinkButton(tr("Back"), this);

    //界面布局
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pSuccessPixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_pSuccessLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_pDetailLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();

    QHBoxLayout *commandLinkButtonLayout = new QHBoxLayout;
    commandLinkButtonLayout->addStretch();
    commandLinkButtonLayout->addWidget(m_pReturnBtn);
    commandLinkButtonLayout->addStretch();

    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_pShowFileBtn, 2);
    buttonHBoxLayout->addStretch(1);

    mainlayout->addLayout(buttonHBoxLayout);
    mainlayout->addLayout(commandLinkButtonLayout);
    mainlayout->setContentsMargins(20, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void SuccessPage::initConnections()
{
    connect(m_pShowFileBtn, &DPushButton::clicked, this, &SuccessPage::signalViewFile);
    connect(m_pReturnBtn, &DCommandLinkButton::clicked, this, &SuccessPage::sigBackButtonClicked);
}


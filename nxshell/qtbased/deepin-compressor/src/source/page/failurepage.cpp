// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "failurepage.h"
#include "customwidget.h"
#include "uitools.h"

#include <DFontSizeManager>

#include <QVBoxLayout>

FailurePage::FailurePage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

FailurePage::~FailurePage()
{

}

void FailurePage::setFailuerDes(const QString &strDes)
{
    m_pFailureLbl->setText(strDes);
}

void FailurePage::setFailureDetail(const QString &strDetail, const QString &strFileName)
{
    if (strFileName.isEmpty()) {
        m_pFileNameLbl->setVisible(false);
    } else {
        m_pFileNameLbl->setVisible(true);
        m_pFileNameLbl->setToolTip(strFileName);
        QFont font = m_pFileNameLbl->font();
        font.setUnderline(true);//下划线
        m_pFileNameLbl->setFont(font);
        QFontMetrics elideFont(font);
        m_pFileNameLbl->setText(elideFont.elidedText(strFileName, Qt::ElideMiddle, 520));
    }
    m_pDetailLbl->setText(strDetail);
}

void FailurePage::setRetryEnable(bool bEnable)
{
    m_pRetrybutton->setEnabled(bEnable);
}

void FailurePage::setFailureInfo(const FailureInfo &failureInfo)
{
    m_failureInfo = failureInfo;
}

FailureInfo FailurePage::getFailureInfo()
{
    return m_failureInfo;
}

void FailurePage::initUI()
{
    //失败图标
    m_pFailurePixmapLbl = new DLabel(this);
    QPixmap failurePixmap = UiTools::renderSVG(":assets/icons/deepin/builtin/icons/compress_fail_128px.svg", QSize(128, 128));
    m_pFailurePixmapLbl->setPixmap(failurePixmap);

    //失败文字
    m_pFailureLbl = new DLabel(this);
    m_pFailureLbl->setText(tr("Extraction failed", "解压失败"));
    DFontSizeManager::instance()->bind(m_pFailureLbl, DFontSizeManager::T5, QFont::DemiBold);
    m_pFailureLbl->setForegroundRole(DPalette::ToolTipText);

    m_pFileNameLbl = new DLabel(this);
    m_pFileNameLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pFileNameLbl, DFontSizeManager::T8);
    m_pFileNameLbl->setVisible(false);

    //失败具体原因
    m_pDetailLbl = new DLabel(this);
    m_pDetailLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pDetailLbl, DFontSizeManager::T8);
    m_pDetailLbl->setText(tr("Damaged file, unable to extract"));

    //重试按钮
    m_pRetrybutton = new CustomPushButton(this);
    m_pRetrybutton->setMinimumSize(340, 36);
    m_pRetrybutton->setText(tr("Retry", "button"));

    //返回按钮
    commandLinkBackButton = new CustomCommandLinkButton(tr("Back"), this);

    //界面布局
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addStretch();
    mainlayout->addWidget(m_pFailurePixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_pFailureLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_pFileNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addWidget(m_pDetailLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    mainlayout->addStretch();
    QHBoxLayout *buttonHBoxLayout = new QHBoxLayout;
    buttonHBoxLayout->addStretch(1);
    buttonHBoxLayout->addWidget(m_pRetrybutton, 2);
    buttonHBoxLayout->addStretch(1);
    QHBoxLayout *commandLinkButtonLayout = new QHBoxLayout;
    commandLinkButtonLayout->addStretch();
    commandLinkButtonLayout->addWidget(commandLinkBackButton);
    commandLinkButtonLayout->addStretch();
    mainlayout->addLayout(buttonHBoxLayout);
    mainlayout->addLayout(commandLinkButtonLayout);
    mainlayout->setContentsMargins(20, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);
}

void FailurePage::initConnections()
{
    connect(m_pRetrybutton, &DPushButton::clicked, this, &FailurePage::sigFailRetry);
    connect(commandLinkBackButton, &DCommandLinkButton::clicked, this, &FailurePage::sigBackButtonClickedOnFail);
}

// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DGuiApplicationHelper>

#include "softwareevaluationofflinewidget.h"
#include "../../core/utils.h"

SoftwareEvaluationOfflineWidget::SoftwareEvaluationOfflineWidget(QWidget *parent)
    :   DWidget(parent)
    ,   m_mainLayout(new QVBoxLayout(this))
    ,   m_labelLayout(new QVBoxLayout(this))
    ,   m_mainWidget(new QWidget(this))
    ,   m_softwareChangeTitleLabel(new DLabel(this))
    ,   m_offlineTipsLabel(new DLabel(this))
{
    initUI();
    initConnections();
}

void SoftwareEvaluationOfflineWidget::initUI()
{
    QPalette palette = this->palette();

    if (isDarkMode())
    {
        palette.setColor(QPalette::Base, "#FF232323");
    }
    else
    {
        palette.setColor(QPalette::Base, "#FFFFFFFF");
    }
    this->setPalette(palette);

    m_softwareChangeTitleLabel->setText(tr("Software Evaluation"));
    m_softwareChangeTitleLabel->setForegroundRole(DPalette::TextTitle);
    DFontSizeManager::instance()->bind(m_softwareChangeTitleLabel, DFontSizeManager::T3, QFont::Bold);

    m_offlineTipsLabel->setText(tr("Your current usage scenario is in a state of no network. We cannot evaluate the software you are using. After the upgrade is complete, the software you are currently installing needs to be reinstalled. If you want to view the details of the applications that can be installed for the upgrade, please upgrade online"));
    m_offlineTipsLabel->setWordWrap(true);
    QPalette pe = m_offlineTipsLabel->palette();
    pe.setColor(m_offlineTipsLabel->foregroundRole(),Qt::blue);
    m_offlineTipsLabel->setPalette(pe);
    DFontSizeManager::instance()->bind(m_offlineTipsLabel, DFontSizeManager::T5, QFont::Medium);
    m_labelLayout->addWidget(m_offlineTipsLabel, Qt::AlignCenter);
    m_mainWidget->setLayout(m_labelLayout);
    m_mainWidget->setFixedSize(572, 160);

    m_mainLayout->setContentsMargins(74, 0, 74, 0);
    m_labelLayout->addSpacing(30);
    m_mainLayout->addWidget(m_softwareChangeTitleLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(30);
    m_mainLayout->addWidget(m_mainWidget, Qt::AlignCenter);
    m_mainLayout->addSpacing(100);
    setLayout(m_mainLayout);
}

void SoftwareEvaluationOfflineWidget::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, [this] {
        QPalette palette = this->palette();
        if (isDarkMode())
        {
            palette.setColor(QPalette::Base, "#FF232323");
        }
        else
        {
            palette.setColor(QPalette::Base, "#FFFFFFFF");
        }
        this->setPalette(palette);
    });
}
